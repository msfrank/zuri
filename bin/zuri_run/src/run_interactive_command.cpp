/* SPDX-License-Identifier: AGPL-3.0-or-later */

#include <lyric_parser/lyric_parser.h>
#include <lyric_runtime/chain_loader.h>
#include <zuri_distributor/runtime_environment.h>
#include <zuri_run/ephemeral_session.h>
#include <zuri_run/fragment_store.h>
#include <zuri_run/log_proto_writer.h>
#include <zuri_run/read_eval_print_loop.h>
#include <zuri_run/run_interactive_command.h>

tempo_utils::Status
zuri_run::run_interactive_command(
    std::shared_ptr<zuri_tooling::EnvironmentConfig> environmentConfig,
    std::shared_ptr<zuri_tooling::BuildToolConfig> buildToolConfig,
    const std::vector<std::string> &mainArgs)
{
    // construct the fragment store
    auto fragmentStore = std::make_shared<FragmentStore>();

    // initialize the parser
    lyric_parser::ParserOptions parserOptions;
    auto parser = std::make_unique<lyric_parser::LyricParser>(parserOptions);

    // construct the runtime environment
    auto environment = environmentConfig->getEnvironment();
    std::shared_ptr<zuri_distributor::RuntimeEnvironment> runtimeEnvironment;
    TU_ASSIGN_OR_RETURN (runtimeEnvironment, zuri_distributor::RuntimeEnvironment::open(
        environment.getEnvironmentDirectory()));

    // construct the loader chain used by the builder and interpreter
    std::vector<std::shared_ptr<lyric_runtime::AbstractLoader>> loaderChain;
    loaderChain.push_back(fragmentStore);
    loaderChain.push_back(runtimeEnvironment->getLoader());
    auto applicationLoader = std::make_shared<lyric_runtime::ChainLoader>(loaderChain);

    // construct the builder used to compile fragments
    lyric_build::BuilderOptions builderOptions;
    builderOptions.cacheMode = lyric_build::CacheMode::InMemory;
    builderOptions.fallbackLoader = applicationLoader;
    builderOptions.virtualFilesystem = fragmentStore;
    auto builder = std::make_unique<lyric_build::LyricBuilder>(
        std::filesystem::current_path(), buildToolConfig->getTaskSettings(), builderOptions);

    // initialize the builder
    TU_RETURN_IF_NOT_OK (builder->configure());

    // construct the interpreter state
    lyric_runtime::InterpreterStateOptions interpreterOptions;
    interpreterOptions.mainArguments = mainArgs;
    std::shared_ptr<lyric_runtime::InterpreterState> interpreterState;
    TU_ASSIGN_OR_RETURN(interpreterState, lyric_runtime::InterpreterState::create(
        builder->getBootstrapLoader(), applicationLoader, interpreterOptions));

    // handle log protocol messages
    auto *portMultiplexer = interpreterState->portMultiplexer();
    auto logProtoUrl = tempo_utils::Url::fromString("dev.zuri.proto:log");
    std::shared_ptr<lyric_runtime::DuplexPort> logPort;
    TU_ASSIGN_OR_RETURN (logPort, portMultiplexer->registerPort(logProtoUrl));
    LogProtoWriter logProtoWriter(false);
    TU_RETURN_IF_NOT_OK (logPort->attach(&logProtoWriter));

    auto sessionId = tempo_utils::UUID::randomUUID().toString();

    // construct the session
    auto ephemeralSession = std::make_shared<EphemeralSession>(sessionId,
        std::move(parser), std::move(builder), fragmentStore, interpreterState);

    // construct and configure the repl
    ReadEvalPrintLoop repl(ephemeralSession);
    TU_RETURN_IF_NOT_OK (repl.configure());

    // hand over control to the repl
    TU_RETURN_IF_NOT_OK (repl.run());

    return repl.cleanup();
}
