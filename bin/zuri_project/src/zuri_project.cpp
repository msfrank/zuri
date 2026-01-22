/* SPDX-License-Identifier: AGPL-3.0-or-later */

#include <tempo_command/command_config.h>
#include <tempo_command/command_help.h>
#include <tempo_command/command_parser.h>
#include <tempo_command/command_tokenizer.h>
#include <tempo_config/base_conversions.h>
#include <tempo_utils/uuid.h>
#include <zuri_project/project_add_command.h>
#include <zuri_project/project_new_command.h>
#include <zuri_project/zuri_project.h>

tempo_utils::Status
zuri_project::zuri_project(int argc, const char *argv[])
{
    tempo_config::PathParser workspaceRootParser(std::filesystem::path{});
    tempo_config::BooleanParser noHomeParser(false);
    tempo_config::BooleanParser colorizeOutputParser(false);
    tempo_config::IntegerParser verboseParser(0);
    tempo_config::IntegerParser quietParser(0);
    tempo_config::BooleanParser silentParser(false);

    std::vector<tempo_command::Default> defaults = {
        {"noHome", "ignore Zuri home"},
        {"colorizeOutput", "Display colorized output"},
        {"verbose", "Display verbose output (specify twice for even more verbose output)"},
        {"quiet", "Display warnings and errors only (specify twice for errors only)"},
        {"silent", "Suppress all output"},
    };

    const std::vector<tempo_command::Grouping> groupings = {
        {"noHome", {"--no-home"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"colorizeOutput", {"-c", "--colorize"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"verbose", {"-v"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"quiet", {"-q"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"silent", {"-s", "--silent"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"help", {"-h", "--help"}, tempo_command::GroupingType::HELP_FLAG},
        {"version", {"--version"}, tempo_command::GroupingType::VERSION_FLAG},
    };

    enum Subcommands {
        New,
        Add,
        NUM_SUBCOMMANDS,
    };
    std::vector<tempo_command::Subcommand> subcommands(NUM_SUBCOMMANDS);
    subcommands[New] = {"new", "Create a new project"};
    subcommands[Add] = {"add", "Add a new target"};

    const std::vector<tempo_command::Mapping> optMappings = {
        {tempo_command::MappingType::TRUE_IF_INSTANCE, "noHome"},
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
                display_help_and_exit({"zuri-project"},
                    "Manage Zuri projects",
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

    bool colorizeOutput;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(colorizeOutput, colorizeOutputParser,
        commandConfig, "colorizeOutput"));

    // initialize logging
    tempo_utils::init_logging(logging);

    TU_LOG_V << "command config:\n" << tempo_command::command_config_to_string(commandConfig);

    // determine whether to load home
    bool noHome;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(noHome, noHomeParser,
        commandConfig, "noHome"));

    // load the distribution
    zuri_tooling::Distribution distribution;
    TU_ASSIGN_OR_RETURN (distribution, zuri_tooling::Distribution::open());

    // open the home if needed
    zuri_tooling::Home home;
    if (!noHome) {
        TU_ASSIGN_OR_RETURN (home, zuri_tooling::Home::open(/* ignoreMissing= */ true));
    }

    // load core config
    std::shared_ptr<zuri_tooling::CoreConfig> coreConfig;
    TU_ASSIGN_OR_RETURN (coreConfig, zuri_tooling::CoreConfig::load(distribution, home));

    switch (selected) {
        case New:
            return project_new_command(coreConfig, tokens);
        case Add:
            return project_add_command(coreConfig, tokens);
        default:
            return tempo_command::CommandStatus::forCondition(
                tempo_command::CommandCondition::kCommandInvariant, "unexpected subcommand");
    }
}
