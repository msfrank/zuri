/* SPDX-License-Identifier: AGPL-3.0-or-later */

#include <tempo_command/command.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <tempo_utils/uuid.h>
#include <zuri_run/read_eval_print_loop.h>
#include <zuri_run/run_interactive_command.h>
#include <zuri_run/run_package_command.h>
#include <zuri_run/run_result.h>
#include <zuri_run/zuri_run.h>
#include <zuri_tooling/environment_config.h>
#include <zuri_tooling/project_config.h>

struct MainPackageOrStdin {
    enum class Type {
        Invalid,
        MainPackagePath,
        Stdin,
    };
    Type type;
    std::filesystem::path mainPackagePath;
};

class MainPackageOrStdinParser : public tempo_config::AbstractConverter<MainPackageOrStdin> {
public:
    tempo_utils::Status convertValue(
        const tempo_config::ConfigNode &node,
        MainPackageOrStdin &value) const override
    {
        value.type = MainPackageOrStdin::Type::Invalid;
        if (node.isNil()) {
            value.type = MainPackageOrStdin::Type::Stdin;
            return {};
        }
        if (node.getNodeType() != tempo_config::ConfigNodeType::kValue)
            return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kWrongType,
                "expected Value node but found {}", config_node_type_to_string(node.getNodeType()));
        auto v = node.toValue().getValue();
        if (v == "-") {
            value.type = MainPackageOrStdin::Type::Stdin;
            return {};
        }
        tempo_config::PathParser mainPackageParser;
        TU_RETURN_IF_NOT_OK (mainPackageParser.convertValue(node, value.mainPackagePath));
        value.type = MainPackageOrStdin::Type::MainPackagePath;
        return {};
    }
};

tempo_utils::Status
zuri_run::zuri_run(int argc, const char *argv[])
{
    tempo_config::PathParser searchStartParser(std::filesystem::current_path());
    tempo_config::BooleanParser noHomeParser(false);
    tempo_config::BooleanParser colorizeOutputParser(false);
    tempo_config::IntegerParser verboseParser(0);
    tempo_config::IntegerParser quietParser(0);
    tempo_config::BooleanParser silentParser(false);
    MainPackageOrStdinParser mainPackageOrStdinParser;
    tempo_config::StringParser mainArgParser;
    tempo_config::SeqTParser mainArgsParser(&mainArgParser);

    tempo_command::Command command("zuri-run");

    command.addArgument("mainPackageOrStdin", "MAIN-PKG | '-'", tempo_command::MappingType::ZERO_OR_ONE_INSTANCE,
        "Main package path, or '-' to run interactively");
    command.addArgument("mainArgs", "ARGS", tempo_command::MappingType::ANY_INSTANCES,
        "List of arguments to pass to the program");
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
        "Run a Zuri program");
    command.addVersionOption("version", {"--version"}, PROJECT_VERSION);

    TU_RETURN_IF_NOT_OK (command.parse(argc - 1, &argv[1]));

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

    // determine whether to run a specified package or run interactively
    MainPackageOrStdin mainPackageOrStdin;
    TU_RETURN_IF_NOT_OK(command.convert(mainPackageOrStdin, mainPackageOrStdinParser, "mainPackageOrStdin"));

    // determine the program arguments
    std::vector<std::string> mainArgs;
    TU_RETURN_IF_NOT_OK(command.convert(mainArgs, mainArgsParser, "mainArgs"));

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

    // open the environment
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

    // load the build tool config from the project or use the core defaults
    std::shared_ptr<zuri_tooling::BuildToolConfig> buildConfig;
    if (project.isValid()) {
        std::shared_ptr<zuri_tooling::ProjectConfig> projectConfig;
        TU_ASSIGN_OR_RETURN (projectConfig, zuri_tooling::ProjectConfig::load(project, coreConfig));
        buildConfig = projectConfig->getBuildConfig();
    } else {
        buildConfig = coreConfig->getDefaultBuildConfig();
    }

    // run the package or run interactively
    switch (mainPackageOrStdin.type) {
        case MainPackageOrStdin::Type::MainPackagePath:
            return run_package_command(environmentConfig, mainPackageOrStdin.mainPackagePath, mainArgs);
        case MainPackageOrStdin::Type::Stdin:
            return run_interactive_command(environmentConfig, buildConfig, mainArgs);
        case MainPackageOrStdin::Type::Invalid:
            return RunStatus::forCondition(RunCondition::kRunInvariant,
                "invalid run target");
    }
}
