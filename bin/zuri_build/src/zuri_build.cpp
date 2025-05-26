/* SPDX-License-Identifier: AGPL-3.0-or-later */

#include <iostream>

#include <lyric_build/build_conversions.h>
#include <lyric_build/lyric_builder.h>
#include <tempo_command/command_config.h>
#include <tempo_command/command_help.h>
#include <tempo_command/command_parser.h>
#include <tempo_command/command_tokenizer.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <zuri_build/build_graph.h>
#include <zuri_build/import_store.h>
#include <zuri_build/workspace_config.h>
#include <zuri_build/target_builder.h>
#include <zuri_build/target_store.h>
#include <zuri_build/zuri_build.h>

#include "zuri_build/collect_modules_task.h"

tempo_utils::Status
run_zuri_build(int argc, const char *argv[])
{
    tempo_config::PathParser workspaceRootParser(std::filesystem::current_path());
    tempo_config::PathParser workspaceConfigFileParser(std::filesystem::path{});
    tempo_config::PathParser distributionRootParser(DISTRIBUTION_ROOT);
    tempo_config::PathParser buildRootParser(std::filesystem::path{});
    tempo_config::PathParser installRootParser(std::filesystem::path{});
    tempo_config::IntegerParser jobParallelismParser(0);
    tempo_config::StringParser targetNameParser;
    tempo_config::SeqTParser targetsParser(&targetNameParser, {});
    tempo_config::BooleanParser colorizeOutputParser(false);
    tempo_config::IntegerParser verboseParser(0);
    tempo_config::IntegerParser quietParser(0);
    tempo_config::BooleanParser silentParser(false);

    std::vector<tempo_command::Default> defaults = {
        {"workspaceRoot", workspaceRootParser.getDefault(),
            "Specify an alternative workspace root directory", "DIR"},
        {"workspaceConfigFile", workspaceConfigFileParser.getDefault(),
            "Specify an alternative workspace config file", "FILE"},
        {"buildRoot", buildRootParser.getDefault(),
            "Specify an alternative build root directory", "DIR"},
        {"installRoot", installRootParser.getDefault(),
            "Specify an alternative install root directory", "DIR"},
        {"distributionRoot", distributionRootParser.getDefault(),
            "Specify an alternative distribution root directory", "DIR"},
        {"jobParallelism", {}, "Number of build worker threads", "COUNT"},
        {"colorizeOutput", colorizeOutputParser.getDefault(),
            "Display colorized output"},
        {"verbose", verboseParser.getDefault(),
            "Display verbose output (specify twice for even more verbose output)"},
        {"quiet", quietParser.getDefault(),
            "Display warnings and errors only (specify twice for errors only)"},
        {"silent", silentParser.getDefault(),
            "Suppress all output"},
        {"targets", {}, "Build targets to compute", "TARGET"},
    };

    const std::vector<tempo_command::Grouping> groupings = {
        {"workspaceRoot", {"-W", "--workspace-root"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"workspaceConfigFile", {"--workspace-config-file"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"buildRoot", {"-B", "--build-root"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"installRoot", {"-I", "--install-root"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"distributionRoot", {"--distribution-root"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"jobParallelism", {"-J", "--job-parallelism"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"colorizeOutput", {"-c", "--colorize"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"verbose", {"-v"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"quiet", {"-q"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"silent", {"-s", "--silent"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"help", {"-h", "--help"}, tempo_command::GroupingType::HELP_FLAG},
        {"version", {"--version"}, tempo_command::GroupingType::VERSION_FLAG},
    };

    const std::vector<tempo_command::Mapping> optMappings = {
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "workspaceRoot"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "workspaceConfigFile"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "buildRoot"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "installRoot"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "distributionRoot"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "jobParallelism"},
        {tempo_command::MappingType::TRUE_IF_INSTANCE, "colorizeOutput"},
        {tempo_command::MappingType::COUNT_INSTANCES, "verbose"},
        {tempo_command::MappingType::COUNT_INSTANCES, "quiet"},
        {tempo_command::MappingType::TRUE_IF_INSTANCE, "silent"},
    };

    std::vector<tempo_command::Mapping> argMappings = {
        {tempo_command::MappingType::ANY_INSTANCES, "targets"},
    };

    // parse argv array into a vector of tokens
    auto tokenizeResult = tempo_command::tokenize_argv(argc - 1, &argv[1]);
    if (tokenizeResult.isStatus())
        display_status_and_exit(tokenizeResult.getStatus());
    auto tokens = tokenizeResult.getResult();

    tempo_command::OptionsHash options;
    tempo_command::ArgumentVector arguments;

    // parse global options and get the subcommand
    auto status = tempo_command::parse_completely(tokens, groupings, options, arguments);
    if (status.notOk()) {
        tempo_command::CommandStatus commandStatus;
        if (!status.convertTo(commandStatus))
            return status;
        switch (commandStatus.getCondition()) {
            case tempo_command::CommandCondition::kHelpRequested:
                display_help_and_exit({"zuri-build"},
                    "Build software for the Zuri ecosystem",
                    {}, groupings, optMappings, argMappings, defaults);
            case tempo_command::CommandCondition::kVersionRequested:
                tempo_command::display_version_and_exit(FULL_VERSION);
            default:
                return status;
        }
    }

    // initialize the global config from defaults
    tempo_command::CommandConfig commandConfig = command_config_from_defaults(defaults);

    // convert option to config
    status = tempo_command::convert_options(options, optMappings, commandConfig);
    if (!status.isOk())
        return status;

    // convert arguments to config
    status = tempo_command::convert_arguments(arguments, argMappings, commandConfig);
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
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(logging.colorizeOutput, colorizeOutputParser,
        commandConfig, "colorizeOutput"));

    // initialize logging
    tempo_utils::init_logging(logging);

    TU_LOG_INFO << "command config:\n" << tempo_command::command_config_to_string(commandConfig);

    // determine the workspace root
    std::filesystem::path workspaceRoot;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(workspaceRoot, workspaceRootParser,
        commandConfig, "workspaceRoot"));

    // determine the workspace config file
    std::filesystem::path workspaceConfigFile;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(workspaceConfigFile, workspaceConfigFileParser,
        commandConfig, "workspaceConfigFile"));

    // determine the build root
    std::filesystem::path buildRoot;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(buildRoot, buildRootParser,
        commandConfig, "buildRoot"));

    // determine the install root
    std::filesystem::path installRoot;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(installRoot, installRootParser,
        commandConfig, "installRoot"));

    // determine the distribution root
    std::filesystem::path distributionRoot;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(distributionRoot, distributionRootParser,
        commandConfig, "distributionRoot"));

    // determine the job parallelism
    int jobParallelism;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(jobParallelism, jobParallelismParser,
        commandConfig, "jobParallelism"));

    // determine the list of targets
    std::vector<std::string> targets;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(targets, targetsParser,
        commandConfig, "targets"));

    // if distribution root is relative, then make it absolute
    if (distributionRoot.is_relative()) {
        auto executableDir = std::filesystem::path(argv[0]).parent_path();
        distributionRoot = executableDir / distributionRoot;
    }
    TU_LOG_V << "using distribution root " << distributionRoot;

    // load the workspace config
    std::shared_ptr<tempo_config::WorkspaceConfig> config;
    if (!workspaceConfigFile.empty()) {
        TU_ASSIGN_OR_RETURN (config, load_workspace_config(workspaceConfigFile, distributionRoot));
    } else {
        TU_ASSIGN_OR_RETURN (config, find_workspace_config(workspaceRoot, distributionRoot));
        workspaceRoot = config->getWorkspaceRoot();
    }

    // if build root was not defined in commandConfig, then default to subdirectory of the workspace root
    if (buildRoot.empty()) {
        buildRoot = workspaceRoot / "build";
    }

    auto toolConfig = config->getToolConfig();

    // construct the config store from the workspace
    auto settingsConfig = toolConfig.mapAt("settings").toMap();
    lyric_build::TaskSettings taskSettings(settingsConfig);

    //
    auto importsConfig = toolConfig.mapAt("imports").toMap();
    auto importStore = std::make_shared<ImportStore>(importsConfig);
    TU_RETURN_IF_NOT_OK (importStore->configure());

    //
    auto targetsConfig = toolConfig.mapAt("targets").toMap();
    auto targetStore = std::make_shared<TargetStore>(targetsConfig);
    TU_RETURN_IF_NOT_OK (targetStore->configure());

    //
    std::shared_ptr<BuildGraph> buildGraph;
    TU_ASSIGN_OR_RETURN (buildGraph, BuildGraph::create(targetStore, importStore));

    // set builder options
    lyric_build::BuilderOptions builderOptions;
    builderOptions.buildRoot = buildRoot;
    if (jobParallelism != 0) {
        builderOptions.numThreads = jobParallelism;
    }

    // create task registry and add build task domains
    auto taskRegistry = std::make_shared<lyric_build::TaskRegistry>();
    taskRegistry->registerTaskDomain("collect_modules", new_collect_modules_task);
    builderOptions.taskRegistry = std::move(taskRegistry);

    // construct the builder based on workspace config and config overrides
    lyric_build::LyricBuilder builder(workspaceRoot, taskSettings, builderOptions);
    TU_RETURN_IF_NOT_OK (builder.configure());

    // verify there is at least one target and all targets are defined
    if (targets.empty())
        return tempo_command::CommandStatus::forCondition(
            tempo_command::CommandCondition::kInvalidConfiguration,
            "at least one target must be specified");
    for (const auto &target : targets) {
        if (!targetStore->hasTarget(target))
            return tempo_command::CommandStatus::forCondition(
                tempo_command::CommandCondition::kInvalidConfiguration,
                "unknown target '{}'", target);
    }

    // build each target (and its dependencies) in the order specified on the command line
    for (const auto &target : targets) {
        TargetBuilder targetBuilder(buildGraph, &builder, installRoot);
        TU_RETURN_IF_STATUS (targetBuilder.buildTarget(target));
    }

    return {};
}
