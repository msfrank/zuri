
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

tempo_utils::Status
zuri_zpk::zpk_extract_command(
    std::shared_ptr<zuri_tooling::ZuriConfig> zuriConfig,
    tempo_command::TokenVector &tokens)
{
    tempo_config::PathParser zpkFileParser;

    std::vector<tempo_command::Default> defaults = {
        {"zpkFile", {}, "Package file to inspect", "FILE"},
    };

    const std::vector<tempo_command::Grouping> groupings = {
        {"help", {"-h", "--help"}, tempo_command::GroupingType::HELP_FLAG},
    };

    const std::vector<tempo_command::Mapping> optMappings = {
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

    // initialize the command config from defaults
    tempo_command::CommandConfig commandConfig = tempo_command::command_config_from_defaults(defaults);

    // convert options to config
    TU_RETURN_IF_NOT_OK (tempo_command::convert_options(options, optMappings, commandConfig));

    // convert arguments to config
    TU_RETURN_IF_NOT_OK (tempo_command::convert_arguments(arguments, argMappings, commandConfig));

    // construct command map
    tempo_config::ConfigMap commandMap(commandConfig);

    std::filesystem::path zpkFile;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(zpkFile, zpkFileParser,
        commandConfig, "zpkFile"));

    return {};
}
