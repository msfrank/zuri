/* SPDX-License-Identifier: AGPL-3.0-or-later */

#include <tempo_command/command_config.h>
#include <tempo_command/command_help.h>
#include <tempo_command/command_parser.h>
#include <tempo_command/command_tokenizer.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/workspace_config.h>
#include <tempo_utils/uuid.h>
#include <zuri_pkg/zuri_pkg.h>
#include <zuri_tooling/zuri_config.h>

#include "zuri_pkg/pkg_install.h"

tempo_utils::Status
zuri_pkg::zuri_pkg(int argc, const char *argv[])
{
    tempo_config::PathParser workspaceRootParser(std::filesystem::path{});
    tempo_config::PathParser distributionRootParser(DISTRIBUTION_ROOT);
    tempo_config::BooleanParser manageSystemParser(false);
    tempo_config::BooleanParser colorizeOutputParser(false);
    tempo_config::IntegerParser verboseParser(0);
    tempo_config::IntegerParser quietParser(0);
    tempo_config::BooleanParser silentParser(false);

    std::vector<tempo_command::Default> defaults = {
        {"workspaceRoot", workspaceRootParser.getDefault(),
            "Load config from workspace", "DIR"},
        {"distributionRoot", distributionRootParser.getDefault(),
            "Specify an alternative distribution root directory", "DIR"},
        {"manageSystem", manageSystemParser.getDefault(),
            "Manage system cache"},
        {"colorizeOutput", colorizeOutputParser.getDefault(),
            "Display colorized output"},
        {"verbose", verboseParser.getDefault(),
            "Display verbose output (specify twice for even more verbose output)"},
        {"quiet", quietParser.getDefault(),
            "Display warnings and errors only (specify twice for errors only)"},
        {"silent", silentParser.getDefault(),
            "Suppress all output"},
        {"arguments", {}, "List of arguments to pass to the program", "ARGS"},
    };

    const std::vector<tempo_command::Grouping> groupings = {
        {"workspaceRoot", {"-W", "--workspace-root"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"distributionRoot", {"--distribution-root"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"manageSystem", {"-S", "--manage-system"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"colorizeOutput", {"-c", "--colorize"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"verbose", {"-v"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"quiet", {"-q"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"silent", {"-s", "--silent"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"help", {"-h", "--help"}, tempo_command::GroupingType::HELP_FLAG},
        {"version", {"--version"}, tempo_command::GroupingType::VERSION_FLAG},
    };

    enum Subcommands {
        Install,
        Remove,
        Cache,
        NUM_SUBCOMMANDS,
    };
    std::vector<tempo_command::Subcommand> subcommands(NUM_SUBCOMMANDS);
    subcommands[Install] = {"install", "Install packages and their dependencies"};
    subcommands[Remove] = {"remove", "Remove packages"};
    subcommands[Cache] = {"cache", "Manage the package caches"};

    const std::vector<tempo_command::Mapping> optMappings = {
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "workspaceRoot"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "distributionRoot"},
        {tempo_command::MappingType::TRUE_IF_INSTANCE, "manageSystem"},
        {tempo_command::MappingType::TRUE_IF_INSTANCE, "colorizeOutput"},
        {tempo_command::MappingType::COUNT_INSTANCES, "verbose"},
        {tempo_command::MappingType::COUNT_INSTANCES, "quiet"},
        {tempo_command::MappingType::TRUE_IF_INSTANCE, "silent"},
    };

    // parse argv array into a vector of tokens
    tempo_command::TokenVector tokens;
    TU_ASSIGN_OR_RETURN (tokens, tempo_command::tokenize_argv(argc - 1, &argv[1]));

    tempo_command::OptionsHash options;
    int selected;

    // parse options and get the subcommand
    auto status = tempo_command::parse_until_subcommand(tokens, subcommands, groupings, selected, options);
    if (status.notOk()) {
        tempo_command::CommandStatus commandStatus;
        if (!status.convertTo(commandStatus))
            return status;
        switch (commandStatus.getCondition()) {
            case tempo_command::CommandCondition::kHelpRequested:
                display_help_and_exit({"zuri-pkg"},
                    "Manage Zuri packages",
                    subcommands, groupings, optMappings, {}, defaults);
            case tempo_command::CommandCondition::kVersionRequested:
                tempo_command::display_version_and_exit(PROJECT_VERSION);
            default:
                return status;
        }
    }

    // initialize the command config from defaults
    tempo_command::CommandConfig commandConfig = command_config_from_defaults(defaults);

    // convert options to config
    TU_RETURN_IF_NOT_OK (tempo_command::convert_options(options, optMappings, commandConfig));

    // construct command map
    tempo_config::ConfigMap commandMap(commandConfig);

    // configure logging
    tempo_utils::LoggingConfiguration logging = {
        tempo_utils::SeverityFilter::kDefault,
        false,
    };

    bool silent;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(silent, silentParser,
        commandConfig, "silent"));
    if (silent) {
        logging.severityFilter = tempo_utils::SeverityFilter::kSilent;
    } else {
        int verbose, quiet;
        TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(verbose, verboseParser,
            commandConfig, "verbose"));
        TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(quiet, quietParser,
            commandConfig, "quiet"));
        if (verbose && quiet)
            return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
                "cannot specify both -v and -q");
        if (verbose == 1) {
            logging.severityFilter = tempo_utils::SeverityFilter::kVerbose;
        } else if (verbose > 1) {
            logging.severityFilter = tempo_utils::SeverityFilter::kVeryVerbose;
        }
        if (quiet == 1) {
            logging.severityFilter = tempo_utils::SeverityFilter::kWarningsAndErrors;
        } else if (quiet > 1) {
            logging.severityFilter = tempo_utils::SeverityFilter::kErrorsOnly;
        }
    }

    bool manageSystem;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(manageSystem, manageSystemParser,
        commandConfig, "manageSystem"));

    bool colorizeOutput;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(colorizeOutput, colorizeOutputParser,
        commandConfig, "colorizeOutput"));

    // initialize logging
    tempo_utils::init_logging(logging);

    TU_LOG_V << "command config:\n" << tempo_command::command_config_to_string(commandConfig);

    // determine the workspace root
    std::filesystem::path workspaceRoot;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(workspaceRoot, workspaceRootParser,
        commandConfig, "workspaceRoot"));

    // determine the distribution root
    std::filesystem::path distributionRoot;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(distributionRoot, distributionRootParser,
        commandConfig, "distributionRoot"));

    // if distribution root is relative, then make it absolute
    if (!distributionRoot.empty()) {
        if (distributionRoot.is_relative()) {
            auto executableDir = std::filesystem::path(argv[0]).parent_path();
            distributionRoot = executableDir / distributionRoot;
        }
    }

    TU_LOG_V << "using distribution root " << distributionRoot;

    // load zuri config
    std::shared_ptr<zuri_tooling::ZuriConfig> zuriConfig;
    if (!workspaceRoot.empty()) {
        std::filesystem::path workspaceConfigFile;
        TU_ASSIGN_OR_RETURN (workspaceConfigFile, tempo_config::find_workspace_config(workspaceRoot));
        TU_ASSIGN_OR_RETURN (zuriConfig, zuri_tooling::ZuriConfig::forWorkspace(
            workspaceConfigFile, {}, distributionRoot));
    } else {
        TU_ASSIGN_OR_RETURN (zuriConfig, zuri_tooling::ZuriConfig::forUser(
            {}, distributionRoot));
    }

    switch (selected) {
        case Install:
            return pkg_install(distributionRoot, manageSystem, tokens);
        case Remove:
        case Cache:
        default:
            return tempo_command::CommandStatus::forCondition(
                tempo_command::CommandCondition::kCommandInvariant, "unexpected subcommand");
    }
}
