/* SPDX-License-Identifier: AGPL-3.0-or-later */

#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/chain_loader.h>
#include <lyric_common/common_types.h>
#include <tempo_command/command_result.h>
#include <tempo_utils/date_time.h>
#include <tempo_utils/file_utilities.h>
#include <tempo_utils/uuid.h>

#include <zuri/ephemeral_session.h>

EphemeralSession::EphemeralSession(
    const std::string &sessionId,
    std::unique_ptr<lyric_parser::LyricParser> &&parser,
    std::unique_ptr<lyric_build::LyricBuilder> &&builder,
    std::shared_ptr<FragmentStore> fragmentStore,
    std::shared_ptr<lyric_runtime::InterpreterState> interpreterState)
    : m_sessionId(sessionId),
      m_parser(std::move(parser)),
      m_builder(std::move(builder)),
      m_fragmentStore(std::move(fragmentStore)),
      m_interpreterState(std::move(interpreterState))
{
    TU_ASSERT (!m_sessionId.empty());
    TU_ASSERT (m_parser != nullptr);
    TU_ASSERT (m_builder != nullptr);
    TU_ASSERT (m_fragmentStore != nullptr);
    TU_ASSERT (m_interpreterState != nullptr);
}

std::string_view
EphemeralSession::sessionId() const
{
    return m_sessionId;
}

tempo_utils::Result<tempo_utils::Url>
EphemeralSession::parseLine(std::string_view line)
{
    // if the line contains only whitespace, then don't bother parsing
    if (line.find_first_not_of(" \r\n\t\f\v") == std::string::npos)
        return lyric_parser::ParseStatus::forCondition(lyric_parser::ParseCondition::kIncompleteModule);

    // append line to the current code fragment
    m_fragment.append(line);

    auto moduleName = tempo_utils::generate_name("XXXXXXXX");
    auto fragmentUrl = tempo_utils::Url::fromAbsolute("dev.zuri.session", m_sessionId,
        absl::StrCat("/", moduleName, lyric_common::kSourceFileDotSuffix));

    // parse the fragment to determine if the code is complete and syntactically correct
    auto recorder = tempo_tracing::TraceRecorder::create();
    TU_RETURN_IF_STATUS (m_parser->parseModule(m_fragment, fragmentUrl, recorder));

    // write the fragment to the fragment store
    m_fragmentStore->insertFragment(fragmentUrl, m_fragment, tempo_utils::millis_since_epoch());

    // insert the complete fragment and reset the internal string
    m_fragment.clear();

    return fragmentUrl;
}

tempo_utils::Result<lyric_common::ModuleLocation>
EphemeralSession::compileFragment(const tempo_utils::Url &fragmentUrl)
{
    auto moduleLocation = lyric_common::ModuleLocation::fromUrl(fragmentUrl);

    // configure the build task
    lyric_build::TaskId target("compile_module", fragmentUrl.toString());
    absl::flat_hash_map<lyric_build::TaskId,tempo_config::ConfigMap> taskOverrides = {
        { target, tempo_config::ConfigMap({
            //{"envSymbolsPath", {}},
            {"moduleLocation", tempo_config::ConfigValue(moduleLocation.toString())},
            {"skipInstall", tempo_config::ConfigValue("true")},
            })
        }
    };
    lyric_build::TaskSettings overrides({}, {}, taskOverrides);

    // compile the code fragment into an object
    lyric_build::TargetComputationSet targetComputationSet;
    TU_ASSIGN_OR_RETURN (targetComputationSet, m_builder->computeTargets({target}, overrides));
    auto targetComputation = targetComputationSet.getTarget(target);
    auto targetState = targetComputation.getState();

    if (targetState.getStatus() != lyric_build::TaskState::Status::COMPLETED) {
        auto diagnostics = targetComputationSet.getDiagnostics();
        diagnostics->printDiagnostics();
        return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandInvariant,
            "failed to compile fragment");
    }

    auto cache = m_builder->getCache();
    lyric_build::TraceId moduleTrace(targetState.getHash(), target.getDomain(), target.getId());
    auto generation = cache->loadTrace(moduleTrace);
    auto modulePath = moduleLocation.getPath().toFilesystemPath(std::filesystem::path("/"));
    modulePath.replace_extension(lyric_common::kObjectFileSuffix);
    lyric_build::ArtifactId moduleArtifact(generation, targetState.getHash(),
        tempo_utils::UrlPath::fromString(modulePath.string()));

    // read the object from the build cache
    std::shared_ptr<const tempo_utils::ImmutableBytes> content;
    TU_ASSIGN_OR_RETURN (content, cache->loadContentFollowingLinks(moduleArtifact));
    lyric_object::LyricObject object(content);
    if (!object.isValid())
        return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandInvariant,
            "failed to load fragment object");

    m_fragmentStore->insertObject(moduleLocation, object);

    // construct module location based on the source path
    return moduleLocation;
}

tempo_utils::Result<lyric_runtime::DataCell>
EphemeralSession::executeFragment(const lyric_common::ModuleLocation &location)
{
    // initialize the heap and interpreter state
    TU_RETURN_IF_NOT_OK (m_interpreterState->load(location));

    // run the object
    lyric_runtime::BytecodeInterpreter interp(m_interpreterState);
    lyric_runtime::InterpreterExit exit;
    TU_ASSIGN_OR_RETURN (exit, interp.run());
    return exit.mainReturn;
}