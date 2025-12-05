
#include <tempo_command/command_help.h>
#include <tempo_command/command_parser.h>
#include <tempo_config/abstract_converter.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/config_result.h>
#include <tempo_config/container_conversions.h>
#include <tempo_utils/result.h>
#include <zuri_distributor/dependency_selector.h>
#include <zuri_distributor/package_fetcher.h>
#include <zuri_packager/package_specifier.h>
#include <zuri_packager/package_types.h>
#include <zuri_zpk/zpk_extract_command.h>
#include <zuri_zpk/zpk_result.h>

#include "zuri_packager/package_extractor.h"
#include "zuri_packager/package_reader.h"

tempo_utils::Status
zuri_zpk::zpk_extract_command(
    std::shared_ptr<zuri_tooling::ZuriConfig> zuriConfig,
    tempo_command::TokenVector &tokens)
{
    tempo_config::PathParser extractionRootParser(std::filesystem::current_path());
    tempo_config::PathParser workingRootParser(std::filesystem::path{});
    tempo_config::PathParser zpkFileParser;

    std::vector<tempo_command::Default> defaults = {
        {"extractionRoot", "extraction directory", "DIR"},
        {"workingRoot", "working directory", "DIR"},
        {"zpkFile", "Package file to inspect", "FILE"},
    };

    const std::vector<tempo_command::Grouping> groupings = {
        {"extractionRoot", {"-o", "--extraction-root"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"workingRoot", {"-t", "--working-root"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"help", {"-h", "--help"}, tempo_command::GroupingType::HELP_FLAG},
    };

    const std::vector<tempo_command::Mapping> optMappings = {
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "extractionRoot"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "workingRoot"},
    };

    std::vector<tempo_command::Mapping> argMappings = {
        {tempo_command::MappingType::ONE_INSTANCE, "zpkFile"},
    };

    tempo_command::OptionsHash options;
    tempo_command::ArgumentVector arguments;

    // parse global options and arguments
    auto status = tempo_command::parse_completely(tokens, groupings, options, arguments);
    if (status.notOk()) {
        tempo_command::CommandStatus commandStatus;
        if (!status.convertTo(commandStatus))
            return status;
        switch (commandStatus.getCondition()) {
            case tempo_command::CommandCondition::kHelpRequested:
                tempo_command::display_help_and_exit({"zuri-zpk", "extract"},
                    "Extract the specified zpk file",
                    {}, groupings, optMappings, argMappings, defaults);
            case tempo_command::CommandCondition::kVersionRequested:
                tempo_command::display_version_and_exit(PROJECT_VERSION);
            default:
                return status;
        }
    }

    tempo_command::CommandConfig commandConfig;

    // convert options to config
    TU_RETURN_IF_NOT_OK (tempo_command::convert_options(options, optMappings, commandConfig));

    // convert arguments to config
    TU_RETURN_IF_NOT_OK (tempo_command::convert_arguments(arguments, argMappings, commandConfig));

    // construct command map
    tempo_config::ConfigMap commandMap(commandConfig);

    std::filesystem::path extractionRoot;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(extractionRoot, extractionRootParser,
        commandConfig, "extractionRoot"));

    std::filesystem::path workingRoot;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(workingRoot, workingRootParser,
        commandConfig, "workingRoot"));

    std::filesystem::path zpkFile;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(zpkFile, zpkFileParser,
        commandConfig, "zpkFile"));

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
