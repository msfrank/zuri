/* SPDX-License-Identifier: AGPL-3.0-or-later */

#include <tempo_command/command.h>
#include <tempo_config/base_conversions.h>
#include <tempo_utils/uuid.h>
#include <zuri_pkg/pkg_cache_command.h>
#include <zuri_pkg/pkg_install_command.h>
#include <zuri_pkg/zuri_pkg.h>
#include <zuri_tooling/environment_config.h>
#include <zuri_tooling/project_config.h>

tempo_utils::Status
zuri_pkg::zuri_pkg(int argc, const char *argv[])
{
    tempo_config::PathParser searchStartParser(std::filesystem::current_path());
    tempo_config::BooleanParser noHomeParser(false);
    tempo_config::BooleanParser colorizeOutputParser(false);
    tempo_config::IntegerParser verboseParser(0);
    tempo_config::IntegerParser quietParser(0);
    tempo_config::BooleanParser silentParser(false);

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

    tempo_command::Command command("zuri-pkg", subcommands);

    command.addOption("searchStart", {"-S", "--search-start"}, tempo_command::MappingType::ZERO_OR_ONE_INSTANCE,
        "Path to start search for environment", "PATH");
    command.addFlag("noHome", {"--no-home"}, tempo_command::MappingType::ZERO_OR_ONE_INSTANCE,
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
        "Manage Zuri packages");
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

    // determine the search start
    std::filesystem::path searchStart;
    TU_RETURN_IF_NOT_OK(command.convert(searchStart, searchStartParser, "searchStart"));

    // determine whether to load home
    bool noHome;
    TU_RETURN_IF_NOT_OK(command.convert(noHome, noHomeParser, "noHome"));

    // open the distribution
    zuri_tooling::Distribution distribution;
    TU_ASSIGN_OR_RETURN (distribution, zuri_tooling::Distribution::open());

    // open the home if --no-home is not specified
    zuri_tooling::Home home;
    if (!noHome) {
        TU_ASSIGN_OR_RETURN (home, zuri_tooling::Home::open(/* ignoreMissing= */ true));
    }

    // load the core config
    std::shared_ptr<zuri_tooling::CoreConfig> coreConfig;
    TU_ASSIGN_OR_RETURN (coreConfig, zuri_tooling::CoreConfig::load(distribution, home));

    // open the environment runtime
    zuri_tooling::Environment environment;
    zuri_tooling::Project project;
    TU_ASSIGN_OR_RETURN (project, zuri_tooling::Project::find(searchStart));
    if (project.isValid()) {
        TU_ASSIGN_OR_RETURN (environment, zuri_tooling::Environment::open(project.getBuildEnvironmentDirectory()));
    } else {
        TU_ASSIGN_OR_RETURN (environment, zuri_tooling::Environment::find(searchStart));
    }
    if (!environment.isValid()) {
        return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
            "failed to determine the environment");
    }

    // load the environment config
    std::shared_ptr<zuri_tooling::EnvironmentConfig> environmentConfig;
    TU_ASSIGN_OR_RETURN (environmentConfig, zuri_tooling::EnvironmentConfig::load(environment, coreConfig));

    // construct the environment runtime
    std::shared_ptr<zuri_distributor::Runtime> runtime;
    TU_ASSIGN_OR_RETURN (runtime, zuri_distributor::Runtime::open(
        environment.getEnvironmentDirectory()));

    switch (selected) {
        case Cache:
            return pkg_cache_command(environmentConfig, runtime, tokens);
        case Install:
            return pkg_install_command(environmentConfig, runtime, tokens);
        case Remove:
        default:
            return tempo_command::CommandStatus::forCondition(
                tempo_command::CommandCondition::kCommandInvariant, "unexpected subcommand");
    }
}
