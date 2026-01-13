
#include <tempo_command/command_config.h>
#include <tempo_command/command_help.h>
#include <tempo_command/command_parser.h>
#include <zuri_pkg/pkg_cache_command.h>

tempo_utils::Status
zuri_pkg::pkg_cache_command(
    std::shared_ptr<zuri_tooling::EnvironmentConfig> environmentConfig,
    std::shared_ptr<zuri_distributor::Runtime> runtime,
    tempo_command::TokenVector &tokens)
{
    std::vector<tempo_command::Default> defaults = {};

    const std::vector<tempo_command::Grouping> groupings = {
        {"help", {"-h", "--help"}, tempo_command::GroupingType::HELP_FLAG},
    };

    enum Subcommands {
        Info,
        List,
        NUM_SUBCOMMANDS,
    };
    std::vector<tempo_command::Subcommand> subcommands(NUM_SUBCOMMANDS);
    subcommands[Info] = {"info", "Display information about a specified package"};
    subcommands[List] = {"list", "List packages present in the cache"};

    const std::vector<tempo_command::Mapping> optMappings = {
    };

    tempo_command::OptionsHash options;
    int selected;

    // parse global options and arguments
    auto status = tempo_command::parse_until_subcommand(tokens, subcommands, groupings, selected, options);
    if (status.notOk()) {
        tempo_command::CommandStatus commandStatus;
        if (!status.convertTo(commandStatus))
            return status;
        switch (commandStatus.getCondition()) {
            case tempo_command::CommandCondition::kHelpRequested:
                tempo_command::display_help_and_exit({"zuri-pkg", "cache"},
                    "Manage the package caches",
                    subcommands, groupings, optMappings, {}, defaults);
            case tempo_command::CommandCondition::kVersionRequested:
                tempo_command::display_version_and_exit(PROJECT_VERSION);
            default:
                return status;
        }
    }

    tempo_command::CommandConfig commandConfig;

    // convert options to config
    TU_RETURN_IF_NOT_OK (tempo_command::convert_options(options, optMappings, commandConfig));

    // construct command map
    tempo_config::ConfigMap commandMap(commandConfig);

    return {};
}