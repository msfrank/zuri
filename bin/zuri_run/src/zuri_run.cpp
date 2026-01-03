/* SPDX-License-Identifier: AGPL-3.0-or-later */

#include <lyric_runtime/chain_loader.h>
#include <tempo_command/command_config.h>
#include <tempo_command/command_help.h>
#include <tempo_command/command_parser.h>
#include <tempo_command/command_tokenizer.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/workspace_config.h>
#include <tempo_utils/uuid.h>
#include <zuri_run/read_eval_print_loop.h>
#include <zuri_run/run_interactive_command.h>
#include <zuri_run/run_package_command.h>
#include <zuri_run/run_result.h>
#include <zuri_run/zuri_run.h>
#include <zuri_tooling/zuri_config.h>

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
    tempo_config::PathParser workspaceRootParser(std::filesystem::path{});
    tempo_config::BooleanParser noHomeParser(false);
    tempo_config::PathParser sessionRootParser(std::filesystem::path{});
    tempo_config::BooleanParser colorizeOutputParser(false);
    tempo_config::IntegerParser verboseParser(0);
    tempo_config::IntegerParser quietParser(0);
    tempo_config::BooleanParser silentParser(false);
    tempo_config::StringParser sessionIdParser(tempo_utils::UUID::randomUUID().toString());
    MainPackageOrStdinParser mainPackageOrStdinParser;
    tempo_config::StringParser mainArgParser;
    tempo_config::SeqTParser mainArgsParser(&mainArgParser);

    std::vector<tempo_command::Default> defaults = {
        {"workspaceRoot", "Load config from workspace", "DIR"},
        {"noHome", "ignore Zuri home"},
        {"sessionId", "Resume the specified session", "ID"},
        {"colorizeOutput", "Display colorized output"},
        {"verbose", "Display verbose output (specify twice for even more verbose output)"},
        {"quiet", "Display warnings and errors only (specify twice for errors only)"},
        {"silent", "Suppress all output"},
        {"mainPackageOrStdin", "Main package path, or '-' to run interactively", "MAIN-PKG | '-'"},
        {"mainArgs", "List of arguments to pass to the program", "ARGS"},
    };

    const std::vector<tempo_command::Grouping> groupings = {
        {"workspaceRoot", {"-W", "--workspace-root"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"noHome", {"--no-home"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"sessionId", {"-r", "--resume-session"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"colorizeOutput", {"-c", "--colorize"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"verbose", {"-v"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"quiet", {"-q"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"silent", {"-s", "--silent"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"help", {"-h", "--help"}, tempo_command::GroupingType::HELP_FLAG},
        {"version", {"--version"}, tempo_command::GroupingType::VERSION_FLAG},
    };

    const std::vector<tempo_command::Mapping> optMappings = {
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "workspaceRoot"},
        {tempo_command::MappingType::TRUE_IF_INSTANCE, "noHome"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "sessionId"},
        {tempo_command::MappingType::TRUE_IF_INSTANCE, "colorizeOutput"},
        {tempo_command::MappingType::COUNT_INSTANCES, "verbose"},
        {tempo_command::MappingType::COUNT_INSTANCES, "quiet"},
        {tempo_command::MappingType::TRUE_IF_INSTANCE, "silent"},
    };

    std::vector<tempo_command::Mapping> argMappings = {
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "mainPackageOrStdin"},
        {tempo_command::MappingType::ANY_INSTANCES, "mainArgs"},
    };

    // parse argv array into a vector of tokens
    tempo_command::TokenVector tokens;
    TU_ASSIGN_OR_RETURN (tokens, tempo_command::tokenize_argv(argc - 1, &argv[1]));

    tempo_command::OptionsHash options;
    tempo_command::ArgumentVector arguments;

    // parse options and arguments
    auto status = tempo_command::parse_completely(tokens, groupings, options, arguments);
    if (status.notOk()) {
        tempo_command::CommandStatus commandStatus;
        if (!status.convertTo(commandStatus))
            return status;
        switch (commandStatus.getCondition()) {
            case tempo_command::CommandCondition::kHelpRequested:
                display_help_and_exit({"zuri-run"},
                    "Run a Zuri program",
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

    // determine the workspace root
    std::filesystem::path workspaceRoot;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(workspaceRoot, workspaceRootParser,
        commandConfig, "workspaceRoot"));

    // determine the session root
    std::filesystem::path sessionRoot;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(sessionRoot, sessionRootParser,
        commandConfig, "sessionRoot"));

    // determine whether to load home
    bool noHome;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(noHome, noHomeParser,
        commandConfig, "noHome"));

    // determine the session id
    std::string sessionId;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(sessionId, sessionIdParser,
        commandConfig, "sessionId"));

    //
    MainPackageOrStdin mainPackageOrStdin;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(mainPackageOrStdin, mainPackageOrStdinParser,
        commandConfig, "mainPackageOrStdin"));

    //
    std::vector<std::string> mainArgs;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(mainArgs, mainArgsParser,
        commandConfig, "mainArgs"));

    // load the distribution
    zuri_tooling::Distribution distribution;
    TU_ASSIGN_OR_RETURN (distribution, zuri_tooling::Distribution::load());

    TU_LOG_V << "distribution bin dir: " << distribution.getBinDirectory();
    TU_LOG_V << "distribution lib dir: " << distribution.getLibDirectory();
    TU_LOG_V << "distribution packages dir: " << distribution.getPackagesDirectory();
    TU_LOG_V << "distribution config dir: " << distribution.getConfigDirectory();
    TU_LOG_V << "distribution vendor-config dir: " << distribution.getVendorConfigDirectory();

    // open the home if needed
    zuri_tooling::Home home;
    if (!noHome) {
        TU_ASSIGN_OR_RETURN (home, zuri_tooling::Home::open(/* ignoreMissing= */ true));
        if (home.isValid()) {
            TU_LOG_V << "home packages dir: " << home.getPackagesDirectory();
            TU_LOG_V << "home config dir: " << home.getConfigDirectory();
            TU_LOG_V << "home vendor-config dir: " << home.getVendorConfigDirectory();
        } else {
            TU_LOG_V << "no home found";
        }
    }

    // load zuri config
    std::shared_ptr<zuri_tooling::ZuriConfig> zuriConfig;
    if (!workspaceRoot.empty()) {
        std::filesystem::path workspaceConfigFile;
        TU_ASSIGN_OR_RETURN (workspaceConfigFile, tempo_config::find_workspace_config(workspaceRoot));
        TU_ASSIGN_OR_RETURN (zuriConfig, zuri_tooling::ZuriConfig::forWorkspace(
            workspaceConfigFile, home, distribution));
    } else {
        TU_ASSIGN_OR_RETURN (zuriConfig, zuri_tooling::ZuriConfig::forUser(home, distribution));
    }

    //
    switch (mainPackageOrStdin.type) {
        case MainPackageOrStdin::Type::MainPackagePath:
            return run_package_command(zuriConfig, sessionId, mainPackageOrStdin.mainPackagePath, mainArgs);
        case MainPackageOrStdin::Type::Stdin:
            return run_interactive_command(zuriConfig, sessionId, mainArgs);
        case MainPackageOrStdin::Type::Invalid:
            return RunStatus::forCondition(RunCondition::kRunInvariant,
                "invalid run target");
    }
}
