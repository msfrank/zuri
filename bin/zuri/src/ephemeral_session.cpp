/* SPDX-License-Identifier: AGPL-3.0-or-later */

#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/chain_loader.h>
#include <lyric_common/common_types.h>
#include <tempo_command/command_result.h>
#include <tempo_utils/uuid.h>

#include <zuri/ephemeral_session.h>

EphemeralSession::EphemeralSession(
    const std::string &sessionId,
    const lyric_build::ConfigStore &configStore,
    std::shared_ptr<FragmentStore> fragmentStore,
    const lyric_parser::ParserOptions &parserOptions)
    : m_sessionId(sessionId),
      m_configStore(configStore),
      m_fragmentStore(fragmentStore),
      m_parserOptions(parserOptions)
{
    TU_ASSERT (!m_sessionId.empty());
    TU_ASSERT (m_fragmentStore != nullptr);
}

std::string
EphemeralSession::getSessionId() const
{
    return m_sessionId;
}

tempo_utils::Status
EphemeralSession::configure()
{
    if (m_state != nullptr)
        return tempo_utils::GenericStatus::forCondition(
            tempo_utils::GenericCondition::kInternalViolation, "invalid session state");

    // initialize the parser
    m_parser = std::make_unique<lyric_parser::LyricParser>(m_parserOptions);

    // construct the builder used to compile fragments
    lyric_build::BuilderOptions builderOptions;
    builderOptions.cacheMode = lyric_build::CacheMode::InMemory;
    builderOptions.fallbackLoader = m_fragmentStore;
    builderOptions.virtualFilesystem = m_fragmentStore;
    m_builder = std::make_unique<lyric_build::LyricBuilder>(m_configStore, builderOptions);

    // initialize the builder
    auto status = m_builder->configure();
    if (!status.isOk())
        return status;

    lyric_runtime::InterpreterStateOptions options;

    // construct the loader chain
    std::vector<std::shared_ptr<lyric_runtime::AbstractLoader>> loaderChain;
    loaderChain.push_back(m_builder->getBootstrapLoader());
    loaderChain.push_back(m_fragmentStore);
    loaderChain.push_back(m_builder->getPackageLoader());
    options.loader = std::make_shared<lyric_runtime::ChainLoader>(loaderChain);

    // construct the interpreter state
    TU_ASSIGN_OR_RETURN(m_state, lyric_runtime::InterpreterState::create(options));

    // configuration was completed successfully
    return tempo_utils::Status();
}

tempo_utils::Result<std::string>
EphemeralSession::parseLine(const char *data, size_t size)
{
    std::string_view s(data, size);

    // if the line contains only whitespace, then don't bother parsing
    if (s.find_first_not_of(" \r\n\t\f\v") == std::string::npos)
        return lyric_parser::ParseStatus::forCondition(lyric_parser::ParseCondition::kIncompleteModule);

    // append line to the current code fragment
    m_fragment.append(s);

    // parse the fragment to determine if the code is complete and syntactically correct
    auto recorder = tempo_tracing::TraceRecorder::create();
    auto parseResult = m_parser->parseModule(m_fragment, {}, recorder);
    if (parseResult.isStatus())
        return parseResult.getStatus();

    // return the complete fragment and reset the internal string
    auto fragment = m_fragment;
    m_fragment.clear();
    return fragment;
}

tempo_utils::Result<lyric_common::AssemblyLocation>
EphemeralSession::compileFragment(const std::string &fragment)
{
    auto moduleName = tempo_utils::UUID::randomUUID().toString();
    std::filesystem::path modulePath(absl::StrCat("/", m_sessionId, "/", moduleName));
    auto fragmentUrl = tempo_utils::Url::fromOrigin("x.fragment://", modulePath.string());

    // write the fragment to the fragment store
    m_fragmentStore->insertFragment(fragmentUrl, fragment, ToUnixMillis(absl::Now()));
    auto moduleLocation = lyric_common::AssemblyLocation::fromUrl(fragmentUrl);

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

    // compile the code fragment into an object
    auto buildResult = m_builder->computeTargets({target},
        {}, {}, taskOverrides);
    if (buildResult.isStatus())
        return buildResult.getStatus();
    auto targetComputationSet = buildResult.getResult();
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
    modulePath.replace_extension(lyric_common::kAssemblyFileSuffix);
    lyric_build::ArtifactId moduleArtifact(generation, targetState.getHash(), fragmentUrl);

    // read the object from the build cache
    auto loadContentResult = cache->loadContentFollowingLinks(moduleArtifact);
    if (loadContentResult.isStatus())
        return loadContentResult.getStatus();

    lyric_object::LyricObject object(loadContentResult.getResult());
    if (!object.isValid())
        return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandInvariant,
            "failed to load fragment object");

    m_fragmentStore->insertAssembly(moduleLocation, object);

    // construct module location based on the source path
    return moduleLocation;
}

tempo_utils::Result<lyric_runtime::DataCell>
EphemeralSession::executeFragment(const lyric_common::AssemblyLocation &location)
{
    // initialize the heap and interpreter state
    auto initStatus = m_state->reload(location);
    if (!initStatus.isOk())
        return initStatus;

    // run the object
    lyric_runtime::BytecodeInterpreter interp(m_state);
    auto runInterpResult = interp.run();
    if (runInterpResult.isStatus())
        return runInterpResult.getStatus();
    auto exit = runInterpResult.getResult();
    return exit.mainReturn;
}