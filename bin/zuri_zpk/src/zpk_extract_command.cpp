
#include <tempo_command/command.h>
#include <tempo_config/abstract_converter.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <tempo_utils/result.h>
#include <zuri_distributor/dependency_selector.h>
#include <zuri_distributor/package_fetcher.h>
#include <zuri_packager/package_extractor.h>
#include <zuri_packager/package_reader.h>
#include <zuri_zpk/zpk_extract_command.h>
#include <zuri_zpk/zpk_result.h>

tempo_utils::Status
zuri_zpk::zpk_extract_command(
    std::shared_ptr<zuri_tooling::CoreConfig> coreConfig,
    tempo_command::TokenVector &tokens)
{
    tempo_config::PathParser extractionRootParser(std::filesystem::current_path());
    tempo_config::PathParser workingRootParser(std::filesystem::path{});
    tempo_config::PathParser zpkFileParser;

    tempo_command::Command command(std::vector<std::string>{"zuri-zpk", "extract"});

    command.addArgument("zpkFile", "FILE", tempo_command::MappingType::ONE_INSTANCE,
        "Package file to inspect");
    command.addOption("extractionRoot", {"-o", "--extraction-root"}, tempo_command::MappingType::ZERO_OR_ONE_INSTANCE,
        "The extraction directory", "DIR");
    command.addOption("workingRoot", {"-t", "--working-root"}, tempo_command::MappingType::ZERO_OR_ONE_INSTANCE,
        "The working directory", "DIR");
    command.addHelpOption("help", {"-h", "--help"},
        "Extract the specified zpk file");

    // parse global options and arguments
    TU_RETURN_IF_NOT_OK (command.parseCompletely(tokens));

    std::filesystem::path extractionRoot;
    TU_RETURN_IF_NOT_OK (command.convert(extractionRoot, extractionRootParser, "extractionRoot"));

    std::filesystem::path workingRoot;
    TU_RETURN_IF_NOT_OK (command.convert(workingRoot, workingRootParser, "workingRoot"));

    std::filesystem::path zpkFile;
    TU_RETURN_IF_NOT_OK (command.convert(zpkFile, zpkFileParser, "zpkFile"));

    std::shared_ptr<zuri_packager::PackageReader> reader;
    TU_ASSIGN_OR_RETURN (reader, zuri_packager::PackageReader::open(zpkFile));

    zuri_packager::PackageExtractorOptions extractorOptions;
    extractorOptions.destinationRoot = extractionRoot;
    extractorOptions.workingRoot = workingRoot;
    zuri_packager::PackageExtractor extractor(reader, extractorOptions);
    TU_RETURN_IF_NOT_OK (extractor.configure());

    std::filesystem::path packagePath;
    TU_ASSIGN_OR_RETURN (packagePath, extractor.extractPackage());
    TU_CONSOLE_OUT << "extracted " << zpkFile << " to " << packagePath;

    return {};
}
