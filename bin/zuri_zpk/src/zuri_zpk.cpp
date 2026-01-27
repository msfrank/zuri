/* SPDX-License-Identifier: AGPL-3.0-or-later */

#include <tempo_command/command.h>
#include <tempo_config/base_conversions.h>
#include <tempo_utils/uuid.h>
#include <zuri_tooling/core_config.h>
#include <zuri_zpk/zpk_extract_command.h>
#include <zuri_zpk/zpk_inspect_command.h>
#include <zuri_zpk/zuri_zpk.h>

tempo_utils::Status
zuri_zpk::zuri_zpk(int argc, const char *argv[])
{
    tempo_config::PathParser workspaceRootParser(std::filesystem::path{});
    tempo_config::BooleanParser noHomeParser(false);
    tempo_config::BooleanParser manageSystemParser(false);
    tempo_config::BooleanParser colorizeOutputParser(false);
    tempo_config::IntegerParser verboseParser(0);
    tempo_config::IntegerParser quietParser(0);
    tempo_config::BooleanParser silentParser(false);

    enum Subcommands {
        Extract,
        Inspect,
        NUM_SUBCOMMANDS,
    };
    std::vector<tempo_command::Subcommand> subcommands(NUM_SUBCOMMANDS);
    subcommands[Extract] = {"extract", "Extract the specified zpk file"};
    subcommands[Inspect] = {"inspect", "Inspect the contents of a zpk file"};

    tempo_command::Command command("zuri-zpk", subcommands);

    command.addOption("workspaceRoot", {"-W", "--workspace-root"}, tempo_command::MappingType::ZERO_OR_ONE_INSTANCE,
        "Load config from specified workspace", "DIR");
    command.addFlag("noHome", {"--no-home"}, tempo_command::MappingType::TRUE_IF_INSTANCE,
        "Ignore Zuri home");
    command.addFlag("colorizeOutput", {"-c", "--colorize"}, tempo_command::MappingType::TRUE_IF_INSTANCE,
        "Display colorized output");
    command.addFlag("verbose", {"-v"}, tempo_command::MappingType::COUNT_INSTANCES,
        "Display verbose output (specify twice for even more verbose output)");
    command.addFlag("quiet", {"-q"}, tempo_command::MappingType::COUNT_INSTANCES,
        "Display warnings and errors only (specify twice for errors only)");
    command.addFlag("silent", {"-s", "--silent"}, tempo_command::MappingType::TRUE_IF_INSTANCE,
        "Suppress all output");
    command.addHelpOption("help", {"-h", "--help"},
        "Interact with Zuri zpk files");
    command.addVersionOption("version", {"--version"}, PROJECT_VERSION);

    tempo_command::TokenVector tokens;
    TU_ASSIGN_OR_RETURN (tokens, tempo_command::tokenize_argv(argc - 1, &argv[1]));

    int selected;
    TU_RETURN_IF_NOT_OK (command.parseUntilSubcommand(tokens, selected));

    // configure logging
    tempo_utils::LoggingConfiguration logging = {
        tempo_utils::SeverityFilter::kDefault,
        false,
    };

    bool silent;
    TU_RETURN_IF_NOT_OK(command.convert(silent, silentParser, "silent"));
    if (silent) {
        logging.severityFilter = tempo_utils::SeverityFilter::kSilent;
    } else {
        int verbose, quiet;
        TU_RETURN_IF_NOT_OK(command.convert(verbose, verboseParser, "verbose"));
        TU_RETURN_IF_NOT_OK(command.convert(quiet, quietParser, "quiet"));
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
    TU_RETURN_IF_NOT_OK(command.convert(colorizeOutput, colorizeOutputParser, "colorizeOutput"));

    // initialize logging
    tempo_utils::init_logging(logging);

    // determine the workspace root
    std::filesystem::path workspaceRoot;
    TU_RETURN_IF_NOT_OK(command.convert(workspaceRoot, workspaceRootParser, "workspaceRoot"));

    // determine whether to load home
    bool noHome;
    TU_RETURN_IF_NOT_OK(command.convert(noHome, noHomeParser, "noHome"));

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
        case Extract:
            return zpk_extract_command(coreConfig, tokens);
        case Inspect:
            return zpk_inspect_command(coreConfig, tokens);
        default:
            return tempo_command::CommandStatus::forCondition(
                tempo_command::CommandCondition::kCommandInvariant, "unexpected subcommand");
    }
}
