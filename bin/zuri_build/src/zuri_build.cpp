/* SPDX-License-Identifier: AGPL-3.0-or-later */

#include <iostream>

#include <lyric_build/local_filesystem.h>
#include <lyric_build/lyric_builder.h>
#include <lyric_runtime/chain_loader.h>
#include <tempo_command/command.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/config_builder.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/time_conversions.h>
#include <zuri_build/collect_modules_task.h>
#include <zuri_build/import_solver.h>
#include <zuri_build/target_builder.h>
#include <zuri_build/zuri_build.h>
#include <zuri_distributor/distributor_result.h>
#include <zuri_tooling/build_graph.h>
#include <zuri_tooling/tooling_conversions.h>
#include <zuri_tooling/project_config.h>

tempo_utils::Status
zuri_build::zuri_build(int argc, const char *argv[])
{
    tempo_config::PathParser projectRootParser(std::filesystem::path{});
    tempo_config::PathParser projectConfigFileParser(std::filesystem::path{});
    tempo_config::PathParser buildRootParser(std::filesystem::path{});
    tempo_config::PathParser installRootParser(std::filesystem::path{});
    tempo_config::StringParser targetNameParser;
    tempo_config::SeqTParser targetsParser(&targetNameParser, {});
    tempo_config::BooleanParser noHomeParser(false);
    tempo_config::BooleanParser colorizeOutputParser(false);
    tempo_config::IntegerParser verboseParser(0);
    tempo_config::IntegerParser quietParser(0);
    tempo_config::BooleanParser silentParser(false);

    // std::vector<tempo_command::Default> defaults = {
    //     {"projectRoot", "Specify an alternative project root directory", "DIR"},
    //     {"projectConfigFile", "Specify an alternative project config file", "FILE"},
    //     {"noHome", "ignore Zuri home"},
    //     {"buildRoot", "Specify an alternative build root directory", "DIR"},
    //     {"installRoot", "Specify an alternative install root directory", "DIR"},
    //     {"jobParallelism", "Number of build worker threads", "COUNT"},
    //     {"colorizeOutput", "Display colorized output"},
    //     {"verbose", "Display verbose output (specify twice for even more verbose output)"},
    //     {"quiet", "Display warnings and errors only (specify twice for errors only)"},
    //     {"silent", "Suppress all output"},
    //     {"targets", "Build targets to compute", "TARGET"},
    // };
    //
    // const std::vector<tempo_command::Grouping> groupings = {
    //     {"projectRoot", {"-P", "--project-root"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
    //     {"projectConfigFile", {"--project-config-file"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
    //     {"noHome", {"--no-home"}, tempo_command::GroupingType::NO_ARGUMENT},
    //     {"buildRoot", {"-B", "--build-root"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
    //     {"installRoot", {"-I", "--install-root"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
    //     {"jobParallelism", {"-J", "--job-parallelism"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
    //     {"colorizeOutput", {"-c", "--colorize"}, tempo_command::GroupingType::NO_ARGUMENT},
    //     {"verbose", {"-v"}, tempo_command::GroupingType::NO_ARGUMENT},
    //     {"quiet", {"-q"}, tempo_command::GroupingType::NO_ARGUMENT},
    //     {"silent", {"-s", "--silent"}, tempo_command::GroupingType::NO_ARGUMENT},
    //     {"help", {"-h", "--help"}, tempo_command::GroupingType::HELP_FLAG},
    //     {"version", {"--version"}, tempo_command::GroupingType::VERSION_FLAG},
    // };
    //
    // const std::vector<tempo_command::Mapping> optMappings = {
    //     {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "projectRoot"},
    //     {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "projectConfigFile"},
    //     {tempo_command::MappingType::TRUE_IF_INSTANCE, "noHome"},
    //     {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "buildRoot"},
    //     {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "installRoot"},
    //     {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "jobParallelism"},
    //     {tempo_command::MappingType::TRUE_IF_INSTANCE, "colorizeOutput"},
    //     {tempo_command::MappingType::COUNT_INSTANCES, "verbose"},
    //     {tempo_command::MappingType::COUNT_INSTANCES, "quiet"},
    //     {tempo_command::MappingType::TRUE_IF_INSTANCE, "silent"},
    // };
    //
    // std::vector<tempo_command::Mapping> argMappings = {
    //     {tempo_command::MappingType::ANY_INSTANCES, "targets"},
    // };

    tempo_command::Command command("zuri-build");

    command.addArgument("targets", "TARGET", tempo_command::MappingType::ANY_INSTANCES,
        "Build targets to compute");
    command.addOption("projectRoot", {"-P", "--project-root"}, tempo_command::MappingType::ZERO_OR_ONE_INSTANCE,
        "Specify an alternative project root directory", "DIR");
    command.addOption("projectConfigFile", {"--project-config-file"}, tempo_command::MappingType::ZERO_OR_ONE_INSTANCE,
        "Specify an alternative project.config file", "FILE");
    command.addFlag("noHome", {"--no-home"}, tempo_command::MappingType::TRUE_IF_INSTANCE,
        "Ignore Zuri home");
    command.addOption("buildRoot", {"-B", "--build-root"}, tempo_command::MappingType::ZERO_OR_ONE_INSTANCE,
        "Specify an alternative build root directory", "DIR");
    command.addOption("installRoot", {"-I", "--install-root"}, tempo_command::MappingType::ZERO_OR_ONE_INSTANCE,
        "Specify an alternative install root directory", "DIR");
    command.addOption("jobParallelism", {"-J", "--job-parallelism"}, tempo_command::MappingType::ZERO_OR_ONE_INSTANCE,
        "Number of build worker threads", "COUNT");
    command.addFlag("colorizeOutput", {"-c", "--colorize"}, tempo_command::MappingType::TRUE_IF_INSTANCE,
        "Display colorized output");
    command.addFlag("verbose", {"-v"}, tempo_command::MappingType::COUNT_INSTANCES,
        "Display verbose output (specify twice for even more verbose output)");
    command.addFlag("quiet", {"-q"}, tempo_command::MappingType::COUNT_INSTANCES,
        "Display warnings and errors only (specify twice for errors only)");
    command.addFlag("silent", {"-s", "--silent"}, tempo_command::MappingType::TRUE_IF_INSTANCE,
        "Suppress all output");
    command.addHelpOption("help", {"-h", "--help"},
        "Build software for the Zuri ecosystem");
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

    // determine the project root
    std::filesystem::path projectRoot;
    TU_RETURN_IF_NOT_OK(command.convert(projectRoot, projectRootParser, "projectRoot"));

    // determine the project config file
    std::filesystem::path projectConfigFile;
    TU_RETURN_IF_NOT_OK(command.convert(projectConfigFile, projectConfigFileParser, "projectConfigFile"));

    // determine whether to load home
    bool noHome;
    TU_RETURN_IF_NOT_OK(command.convert(noHome, noHomeParser, "noHome"));

    // determine the build root
    std::filesystem::path buildRoot;
    TU_RETURN_IF_NOT_OK(command.convert(buildRoot, buildRootParser, "buildRoot"));

    // determine the install root
    std::filesystem::path installRoot;
    TU_RETURN_IF_NOT_OK(command.convert(installRoot, installRootParser, "installRoot"));

    // determine the list of targets
    std::vector<std::string> targets;
    TU_RETURN_IF_NOT_OK(command.convert(targets, targetsParser, "targets"));

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

    // open the project
    zuri_tooling::Project project;
    if (!projectConfigFile.empty()) {
        TU_ASSIGN_OR_RETURN (project, zuri_tooling::Project::open(projectConfigFile));
    } else if (!projectRoot.empty()) {
        TU_ASSIGN_OR_RETURN (project, zuri_tooling::Project::open(projectRoot));
    } else {
        TU_ASSIGN_OR_RETURN (project, zuri_tooling::Project::find(std::filesystem::current_path()));
    }

    // load the project config
    std::shared_ptr<zuri_tooling::ProjectConfig> projectConfig;
    TU_ASSIGN_OR_RETURN (projectConfig, zuri_tooling::ProjectConfig::load(project, coreConfig));

    // get the environment config for the project
    auto environmentConfig = projectConfig->getEnvironmentConfig();
    auto environment = environmentConfig->getEnvironment();

    // if build root was not specified, then default to the project build directory
    if (buildRoot.empty()) {
        buildRoot = project.getBuildDirectory();
    }

    auto importStore = projectConfig->getImportStore();
    auto targetStore = projectConfig->getTargetStore();
    auto buildToolConfig = projectConfig->getBuildConfig();

    // construct the build graph
    std::shared_ptr<zuri_tooling::BuildGraph> buildGraph;
    TU_ASSIGN_OR_RETURN (buildGraph, zuri_tooling::BuildGraph::create(targetStore, importStore));

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

    // construct the environment runtime
    std::shared_ptr<zuri_distributor::Runtime> runtime;
    TU_ASSIGN_OR_RETURN (runtime, zuri_distributor::Runtime::open(
        environment.getEnvironmentDirectory()));

    // build the task settings
    absl::flat_hash_map<std::string,tempo_config::ConfigNode> globalMap;
    globalMap["runtimeBinDirectory"] = tempo_config::valueNode(runtime->getBinDirectory().string());
    globalMap["runtimeLibDirectory"] = tempo_config::valueNode(runtime->getLibDirectory().string());
    lyric_build::TaskSettings runtimeSettings(globalMap, {}, {});
    auto taskSettings = runtimeSettings.merge(buildToolConfig->getTaskSettings());

    // construct and configure the import solver
    auto importSolver = std::make_shared<ImportSolver>(runtime);
    TU_RETURN_IF_NOT_OK (importSolver->configure());

    lyric_build::BuilderOptions builderOptions;

    // set builder options
    builderOptions.buildRoot = buildRoot;
    builderOptions.cacheMode = buildToolConfig->getCacheMode();
    builderOptions.waitTimeout = buildToolConfig->getWaitTimeout();
    if (project.isLinked()) {
        auto baseDirectory = project.getProjectDirectory();
        TU_ASSIGN_OR_RETURN (builderOptions.virtualFilesystem, lyric_build::LocalFilesystem::create(
            baseDirectory, /* allowSymlinksOutsideBase= */ true));
    }

    // determine the job parallelism
    tempo_config::IntegerParser jobParallelismParser(buildToolConfig->getJobParallelism());
    TU_RETURN_IF_NOT_OK (command.convert(builderOptions.numThreads, jobParallelismParser, "jobParallelism"));

    // create the shortcut resolver
    auto importShortcuts = std::make_shared<lyric_importer::ShortcutResolver>();
    builderOptions.shortcutResolver = importShortcuts;

    // add imports declared in the project
    for (auto it = importStore->importsBegin(); it != importStore->importsEnd(); ++it) {
        const auto &importId = it->first;
        const auto &importEntry = it->second;
        TU_RETURN_IF_NOT_OK (importSolver->addImport(importId, importEntry));
    }

    // add imports declared from package targets
    for (auto it = targetStore->targetsBegin(); it != targetStore->targetsEnd(); it++) {
        const auto &targetName = it->first;
        const auto &targetEntry = it->second;
        if (targetEntry->type == zuri_tooling::TargetEntryType::Package) {
            TU_RETURN_IF_NOT_OK (importSolver->addTarget(targetName, targetEntry));
        }
    }

    // install imports and capture target origins
    absl::flat_hash_map<std::string,tempo_utils::Url> targetBases;
    TU_ASSIGN_OR_RETURN (targetBases, importSolver->installImports(importShortcuts));

    // create task registry and register build task domains
    auto taskRegistry = std::make_shared<lyric_build::TaskRegistry>();
    taskRegistry->registerTaskDomain("collect_modules", new_collect_modules_task);
    builderOptions.taskRegistry = std::move(taskRegistry);

    // set the fallback loader to load from the package cache hierarchy
    builderOptions.fallbackLoader = runtime->getLoader();

    // construct the builder based on runtime, project config, and config overrides
    lyric_build::LyricBuilder builder(projectRoot, taskSettings, builderOptions);
    TU_RETURN_IF_NOT_OK (builder.configure());

    // build each target (and its dependencies) in the order specified on the command line
    TargetBuilder targetBuilder(runtime, buildGraph, &builder, std::move(targetBases), installRoot);
    for (const auto &target : targets) {
        TU_RETURN_IF_STATUS (targetBuilder.buildTarget(target));
    }

    return {};
}
