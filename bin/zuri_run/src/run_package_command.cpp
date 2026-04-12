/* SPDX-License-Identifier: AGPL-3.0-or-later */

#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/chain_loader.h>
#include <tempo_utils/tempdir_maker.h>
#include <tempo_utils/unicode.h>
#include <zuri_distributor/dependency_selector.h>
#include <zuri_distributor/runtime.h>
#include <zuri_packager/package_reader_loader.h>
#include <zuri_run/log_proto_writer.h>
#include <zuri_run/run_package_command.h>
#include <zuri_tooling/package_manager.h>

tempo_utils::Status
zuri_run::run_package_command(
    std::shared_ptr<zuri_tooling::EnvironmentConfig> environmentConfig,
    const std::filesystem::path &mainPackagePath,
    const std::vector<std::string> &mainArgs)
{
    // open the package
    std::shared_ptr<zuri_packager::PackageReader> reader;
    TU_ASSIGN_OR_RETURN (reader, zuri_packager::PackageReader::open(mainPackagePath));

    // determine the entry point
    zuri_packager::PackageSpecifier mainSpecifier;
    TU_ASSIGN_OR_RETURN (mainSpecifier, reader->readPackageSpecifier());
    lyric_common::ModuleLocation programMain;
    TU_ASSIGN_OR_RETURN (programMain, reader->readProgramMain());
    auto mainLocation = lyric_common::ModuleLocation::fromUrl(
        mainSpecifier.toUrl()
            .resolve(programMain.getPath()));
    TU_LOG_V << "main location: " << mainLocation.toString();

    auto tempRoot = std::filesystem::temp_directory_path();

    // construct the package reader loader
    std::shared_ptr<zuri_packager::PackageReaderLoader> packageReaderLoader;
    TU_ASSIGN_OR_RETURN (packageReaderLoader, zuri_packager::PackageReaderLoader::create(reader, tempRoot));

    // construct the bootstrap loader
    auto bootstrapLoader = std::make_shared<lyric_bootstrap::BootstrapLoader>();

    // construct the environment runtime
    auto environment = environmentConfig->getEnvironment();
    std::shared_ptr<zuri_distributor::Runtime> runtime;
    TU_ASSIGN_OR_RETURN (runtime, zuri_distributor::Runtime::open(
        environment.getEnvironmentDirectory()));

    // construct the application loader
    std::vector<std::shared_ptr<lyric_runtime::AbstractLoader>> loaderChain;
    loaderChain.push_back(packageReaderLoader);
    loaderChain.push_back(runtime->getLoader());
    auto applicationLoader = std::make_shared<lyric_runtime::ChainLoader>(loaderChain);

    // construct the log transport
    auto logProtoUrl = tempo_utils::Url::fromString("dev.zuri.proto:log");
    auto logTransport = std::make_shared<LogTransport>(false);
    lyric_runtime::ConnectorPolicy logPolicy;

    // register the log transport
    auto transportRegistry = std::make_shared<lyric_runtime::TransportRegistry>();
    TU_RETURN_IF_NOT_OK (transportRegistry->registerLocalTransport(logProtoUrl, logTransport));

    // construct the interpreter state
    lyric_runtime::InterpreterStateOptions interpreterOptions;
    interpreterOptions.transportRegistry = transportRegistry;
    std::shared_ptr<lyric_runtime::InterpreterState> interpreterState;
    TU_ASSIGN_OR_RETURN(interpreterState, lyric_runtime::InterpreterState::create(
        bootstrapLoader, applicationLoader, interpreterOptions));

    // handle log protocol messages
    auto *portMultiplexer = interpreterState->portMultiplexer();
    TU_RETURN_IF_NOT_OK (portMultiplexer->registerConnector(logProtoUrl, logPolicy));

    // initialize the heap and interpreter state
    TU_RETURN_IF_NOT_OK (interpreterState->load(mainLocation, mainArgs));

    // run the program
    lyric_runtime::BytecodeInterpreter interp(interpreterState);
    lyric_runtime::InterpreterExit exit;
    TU_ASSIGN_OR_RETURN (exit, interp.run());

    // print the return value
    TU_CONSOLE_OUT << " ---> " << exit.mainReturn;

    return {};
}
