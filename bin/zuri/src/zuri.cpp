/* SPDX-License-Identifier: AGPL-3.0-or-later */

#include <tempo_command/command_config.h>
#include <tempo_command/command_help.h>
#include <tempo_command/command_parser.h>
#include <tempo_command/command_tokenizer.h>
#include <tempo_config/base_conversions.h>
#include <tempo_utils/uuid.h>
#include <zuri/load_config.h>
#include <zuri/read_eval_print_loop.h>

tempo_utils::Status
run_zuri(int argc, const char *argv[])
{

    tempo_config::PathParser workspaceRootParser(std::filesystem::path{});
    tempo_config::PathParser distributionRootParser(DISTRIBUTION_ROOT);
    tempo_config::PathParser sessionRootParser(std::filesystem::path{});
    tempo_config::BooleanParser colorizeOutputParser(false);
    tempo_config::IntegerParser verboseParser(0);
    tempo_config::IntegerParser quietParser(0);
    tempo_config::BooleanParser silentParser(false);

    tempo_config::StringParser sessionIdParser(std::string{});

    std::vector<tempo_command::Default> shellDefaults = {
        {"workspaceRoot", workspaceRootParser.getDefault(),
            "Load config from workspace", "DIR"},
        {"distributionRoot", distributionRootParser.getDefault(),
            "Specify an alternative distribution root directory", "DIR"},
        {"sessionId", {}, "Resume the specified session", "ID"},
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

    const std::vector<tempo_command::Grouping> shellGroupings = {
        {"workspaceRoot", {"-W", "--workspace-root"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"distributionRoot", {"--distribution-root"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
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
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "distributionRoot"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "sessionId"},
        {tempo_command::MappingType::TRUE_IF_INSTANCE, "colorizeOutput"},
        {tempo_command::MappingType::COUNT_INSTANCES, "verbose"},
        {tempo_command::MappingType::COUNT_INSTANCES, "quiet"},
        {tempo_command::MappingType::TRUE_IF_INSTANCE, "silent"},
    };

    std::vector<tempo_command::Mapping> argMappings = {
        {tempo_command::MappingType::ANY_INSTANCES, "arguments"},
    };

    // parse argv array into a vector of tokens
    auto tokenizeResult = tempo_command::tokenize_argv(argc - 1, &argv[1]);
    if (tokenizeResult.isStatus())
        display_status_and_exit(tokenizeResult.getStatus());
    auto tokens = tokenizeResult.getResult();

    tempo_command::OptionsHash shellOptions;
    tempo_command::ArgumentVector shellArguments;

    // parse shell options and get the subcommand
    auto status = tempo_command::parse_completely(tokens, shellGroupings, shellOptions, shellArguments);
    if (status.notOk()) {
        tempo_command::CommandStatus commandStatus;
        if (!status.convertTo(commandStatus))
            return status;
        switch (commandStatus.getCondition()) {
            case tempo_command::CommandCondition::kHelpRequested:
                display_help_and_exit({"zuri"},
                    "Run the zuri shell",
                    {}, shellGroupings, optMappings, argMappings, shellDefaults);
            case tempo_command::CommandCondition::kVersionRequested:
                tempo_command::display_version_and_exit(FULL_VERSION);
            default:
                return status;
        }
    }

    // initialize the shell config from defaults
    tempo_command::CommandConfig shellConfig = command_config_from_defaults(shellDefaults);

    // convert options to config
    status = tempo_command::convert_options(shellOptions, optMappings, shellConfig);
    if (!status.isOk())
        return status;

    // convert arguments to config
    status = tempo_command::convert_arguments(shellArguments, argMappings, shellConfig);
    if (!status.isOk())
        return status;

    // configure logging
    tempo_utils::LoggingConfiguration logging = {
        tempo_utils::SeverityFilter::kDefault,
        true,
        false,
        false,
    };

    bool silent;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(silent, silentParser,
        shellConfig, "silent"));
    if (silent) {
        logging.severityFilter = tempo_utils::SeverityFilter::kSilent;
    } else {
        int verbose, quiet;
        TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(verbose, verboseParser,
            shellConfig, "verbose"));
        TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(quiet, quietParser,
            shellConfig, "quiet"));
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
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(logging.colorizeOutput, colorizeOutputParser,
        shellConfig, "colorizeOutput"));

    // initialize logging
    tempo_utils::init_logging(logging);

    TU_LOG_INFO << "shell config:\n" << tempo_command::command_config_to_string(shellConfig);

    // determine the workspace root
    std::filesystem::path workspaceRoot;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(workspaceRoot, workspaceRootParser,
        shellConfig, "workspaceRoot"));

    // determine the session root
    std::filesystem::path sessionRoot;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(sessionRoot, sessionRootParser,
        shellConfig, "sessionRoot"));

    // determine the distribution root
    std::filesystem::path distributionRoot;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(distributionRoot, distributionRootParser,
        shellConfig, "distributionRoot"));

    // determine the session id
    std::string sessionIdString;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(sessionIdString, sessionIdParser,
        shellConfig, "sessionId"));

    // if distribution root is relative, then make it absolute
    if (distributionRoot.is_relative()) {
        auto executableDir = std::filesystem::path(argv[0]).parent_path();
        distributionRoot = executableDir / distributionRoot;
    }
    TU_LOG_V << "using distribution root " << distributionRoot;

    tempo_config::ConfigMap toolConfig;
    tempo_config::ConfigMap vendorConfig;

    // load tool config and vendor config
    if (!workspaceRoot.empty()) {
        auto loadConfigResult = load_workspace_config(workspaceRoot, distributionRoot);
        if (loadConfigResult.isStatus())
            return loadConfigResult.getStatus();
        auto config = loadConfigResult.getResult();
        toolConfig = config->getToolConfig();
        vendorConfig = config->getVendorConfig();
    } else {
        auto loadConfigResult = load_program_config(distributionRoot);
        if (loadConfigResult.isStatus())
            return loadConfigResult.getStatus();
        auto config = loadConfigResult.getResult();
        toolConfig = config->getToolConfig();
        vendorConfig = config->getVendorConfig();
    }

    // construct the build config store
    auto buildNode = toolConfig.mapAt("build");
    lyric_build::ConfigStore configStore(buildNode.toMap(), vendorConfig);

    // construct the fragment store
    auto fragmentStore = std::make_shared<FragmentStore>();

    // construct the session
    std::unique_ptr<EphemeralSession> ephemeralSession;
    if (!sessionIdString.empty()) {
        ephemeralSession = std::make_unique<EphemeralSession>(
            sessionIdString, configStore, fragmentStore);
    } else {
        ephemeralSession = std::make_unique<EphemeralSession>(
            tempo_utils::UUID::randomUUID().toString(), configStore, fragmentStore);
    }

    // configure the session
    TU_RETURN_IF_NOT_OK (ephemeralSession->configure());

    // construct and configure the repl
    ReadEvalPrintLoop repl(ephemeralSession.get());
    TU_RETURN_IF_NOT_OK (repl.configure());

    // hand over control to the repl
    return repl.run();
}