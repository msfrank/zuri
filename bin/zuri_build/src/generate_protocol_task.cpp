
#include <lyric_assembler/assembler_result.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/namespace_symbol.h>
#include <lyric_assembler/object_root.h>
#include <lyric_assembler/object_state.h>
#include <lyric_assembler/protocol_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_bootstrap/bootstrap_helpers.h>
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_result.h>
#include <lyric_build/task_hasher.h>
#include <lyric_build/dependency_loader.h>
#include <lyric_build/metadata_writer.h>
#include <lyric_build/task_utils.h>
#include <lyric_common/common_conversions.h>
#include <lyric_common/common_types.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_config/enum_conversions.h>
#include <zuri_build/generate_protocol_task.h>

zuri_build::GenerateProtocolTask::GenerateProtocolTask(
    const lyric_build::BuildGeneration &generation,
    const lyric_build::TaskKey &key,
    std::weak_ptr<lyric_build::BuildState> buildState,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, std::move(buildState), std::move(span)),
      m_protocolType(lyric_object::PortType::Invalid),
      m_communicationDirection(lyric_object::CommunicationType::Invalid)
{
}

tempo_utils::Status
zuri_build::GenerateProtocolTask::configureTask(const lyric_build::TaskSettings &taskSettings)
{
    auto taskId = getId();
    auto settings = taskSettings.merge(lyric_build::TaskSettings({}, {}, {{taskId, getParams()}}));

    auto modulePath = tempo_utils::UrlPath::fromString(taskId.getId());
    if (!modulePath.isValid())
        return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kInvalidConfiguration,
            "task key id {} is not a valid relative module location", taskId.getId());

    m_moduleLocation = lyric_common::ModuleLocation::fromString(modulePath.toString());

    // set the prelude location
    lyric_common::ModuleLocationParser preludeLocationParser(lyric_bootstrap::preludeLocation());
    TU_RETURN_IF_NOT_OK(parse_config(m_preludeLocation, preludeLocationParser,
        settings, taskId, "preludeLocation"));

    //
    // config below comes only from the task section, it is not resolved from domain or global sections
    //

    auto taskSection = settings.getTaskSection(taskId);

    tempo_config::StringParser nameParser;
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(m_protocolName, nameParser, taskSection, "protocolName"));

    tempo_config::EnumTParser<lyric_object::PortType> typeParser(
        {
            {"Accept", lyric_object::PortType::Accept},
            {"Connect", lyric_object::PortType::Connect},
        });
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(m_protocolType, typeParser, taskSection, "protocolType"));

    tempo_config::EnumTParser<lyric_object::CommunicationType> directionParser(
        {
            {"Send", lyric_object::CommunicationType::Send},
            {"Receive", lyric_object::CommunicationType::Receive},
            {"SendAndReceive", lyric_object::CommunicationType::SendAndReceive},
        });
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(m_communicationDirection, directionParser,
        taskSection, "communicationDirection"));

    switch (m_communicationDirection) {
        case lyric_object::CommunicationType::Send:
        case lyric_object::CommunicationType::SendAndReceive: {
            lyric_common::SymbolUrlParser sendSymbolParser;
            TU_RETURN_IF_NOT_OK (tempo_config::parse_config(m_sendSymbol, sendSymbolParser,
                taskSection, "sends"));
            if (m_sendSymbol.isRelative()) {
                m_sendSymbol = lyric_common::SymbolUrl(m_preludeLocation, m_sendSymbol.getSymbolPath());
            }
            break;
        }
        default:
            break;
    }

    switch (m_communicationDirection) {
        case lyric_object::CommunicationType::Receive:
        case lyric_object::CommunicationType::SendAndReceive: {
            lyric_common::SymbolUrlParser receiveSymbolParser;
            TU_RETURN_IF_NOT_OK (tempo_config::parse_config(m_receiveSymbol, receiveSymbolParser,
                taskSection, "receives"));
            if (m_receiveSymbol.isRelative()) {
                m_receiveSymbol = lyric_common::SymbolUrl(m_preludeLocation, m_receiveSymbol.getSymbolPath());
            }
            break;
        }
        default:
            break;
    }

    return {};
}

tempo_utils::Status
zuri_build::GenerateProtocolTask::deduplicateTask(lyric_build::TaskHash &taskHash)
{
    lyric_build::TaskHasher taskHasher(getKey());

    taskHasher.hashValue(m_moduleLocation.toString());
    taskHasher.hashValue(m_preludeLocation.toString());
    taskHasher.hashValue(m_protocolName);
    taskHasher.hashValue(static_cast<tu_int64>(m_protocolType));
    taskHasher.hashValue(static_cast<tu_int64>(m_communicationDirection));
    taskHasher.hashValue(m_sendSymbol.toString());
    taskHasher.hashValue(m_receiveSymbol.toString());
    taskHash = taskHasher.finish();

    return {};
}

tempo_utils::Status
zuri_build::GenerateProtocolTask::runTask(lyric_build::TempDirectory *tempDirectory)
{
    logInfo("assembling protocol module {}", m_moduleLocation.toString());

    // define the module origin
    auto origin = lyric_common::ModuleLocation::fromString(
        absl::StrCat("dev.zuri.build://", getGeneration().toString()));

    auto buildState = getBuildState();
    auto artifactCache = buildState->getArtifactCache();

    // construct the local module cache
    std::shared_ptr<lyric_runtime::AbstractLoader> dependencyLoader;
    TU_ASSIGN_OR_RETURN (dependencyLoader, lyric_build::DependencyLoader::create(origin, this,  tempDirectory));
    auto localModuleCache = lyric_importer::ModuleCache::create(dependencyLoader);

    auto sharedModuleCache = buildState->getSharedModuleCache();
    auto shortcutResolver = buildState->getShortcutResolver();

    lyric_assembler::ObjectStateOptions options;
    options.preludeLocation = m_preludeLocation;

    // configure object state
    lyric_assembler::ObjectState objectState(m_moduleLocation, origin, localModuleCache, sharedModuleCache,
        shortcutResolver, options);
    lyric_assembler::ObjectRoot *objectRoot;
    TU_ASSIGN_OR_RETURN (objectRoot, objectState.defineRoot());

    // // define empty entry call
    // auto *entryCall = objectRoot->entryCall();
    // auto *proc = entryCall->callProc();
    // auto *code = proc->procFragment();
    // TU_RETURN_IF_NOT_OK (code->returnToCaller());

    // define the protocol
    auto *symbolCache = objectState.symbolCache();
    auto *fundamentalCache = objectState.fundamentalCache();
    auto UndefType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Undef);

    lyric_common::TypeDef sendType;
    if (m_sendSymbol.isValid()) {
        lyric_assembler::AbstractSymbol *sendSymbol;
        TU_ASSIGN_OR_RETURN (sendSymbol, symbolCache->getOrImportSymbol(m_sendSymbol));
        sendType = sendSymbol->getTypeDef();
    } else {
        sendType = UndefType;
    }

    lyric_common::TypeDef receiveType;
    if (m_receiveSymbol.isValid()) {
        lyric_assembler::AbstractSymbol *receiveSymbol;
        TU_ASSIGN_OR_RETURN (receiveSymbol, symbolCache->getOrImportSymbol(m_receiveSymbol));
        receiveType = receiveSymbol->getTypeDef();
    } else {
        receiveType = UndefType;
    }

    auto *block = objectRoot->rootBlock();
    lyric_assembler::ProtocolSymbol *protocolSymbol;
    TU_ASSIGN_OR_RETURN (protocolSymbol, block->declareProtocol(m_protocolName, /* isHidden= */ false,
        sendType, receiveType, m_protocolType, m_communicationDirection));

    // add protocol to namespace
    auto *globalNs = objectRoot->globalNamespace();
    TU_RETURN_IF_NOT_OK (globalNs->putTarget(protocolSymbol->getSymbolUrl()));

    // generate the object containing the protocol
    lyric_object::LyricObject object;
    TU_ASSIGN_OR_RETURN (object, objectState.toObject());

    // declare the object artifact path
    tempo_utils::UrlPath objectArtifactPath;
    TU_ASSIGN_OR_RETURN (objectArtifactPath, lyric_build::convert_module_location_to_artifact_path(
        m_moduleLocation, lyric_common::kObjectFileDotSuffix));

    // construct the object metadata
    lyric_build::MetadataWriter objectWriter;
    TU_RETURN_IF_NOT_OK (objectWriter.configure());
    objectWriter.putAttr(lyric_build::kLyricBuildContentType, std::string(lyric_common::kObjectContentType));
    objectWriter.putAttr(lyric_build::kLyricBuildModuleLocation, m_moduleLocation);
    lyric_build::LyricMetadata objectMetadata;
    TU_ASSIGN_OR_RETURN (objectMetadata, objectWriter.toMetadata());

    // store the object content in the cache
    auto objectBytes = object.toBytes();
    TU_RETURN_IF_NOT_OK (storeArtifact(objectArtifactPath, objectBytes, objectMetadata));

    logInfo("stored protocol object {}", objectArtifactPath.toString());

    return {};
}

lyric_build::BaseTask *
zuri_build::new_generate_protocol_task(
    const lyric_build::BuildGeneration &generation,
    const lyric_build::TaskKey &key,
    std::weak_ptr<lyric_build::BuildState> buildState,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new GenerateProtocolTask(generation, key, std::move(buildState), std::move(span));
}