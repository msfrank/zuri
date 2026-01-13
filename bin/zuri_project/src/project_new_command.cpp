
#include <tempo_command/command_help.h>
#include <tempo_command/command_parser.h>
#include <tempo_config/abstract_converter.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <tempo_utils/result.h>
#include <zuri_distributor/dependency_selector.h>
#include <zuri_distributor/package_fetcher.h>
#include <zuri_project/project_new_command.h>
#include <zuri_project/project_result.h>

#include "zuri_tooling/project.h"

tempo_utils::Status
zuri_project::project_new_command(
    std::shared_ptr<zuri_tooling::CoreConfig> coreConfig,
    tempo_command::TokenVector &tokens)
{
    tempo_config::PathParser projectConfigFileParser(std::filesystem::path{});
    tempo_config::PathParser pathParser;
    tempo_config::SeqTParser copyTargetListParser(&pathParser, {});
    tempo_config::PathParser copyTargetsDirectoryParser(std::filesystem::path{});
    tempo_config::SeqTParser copyConfigListParser(&pathParser, {});
    tempo_config::PathParser copyConfigDirectoryParser(std::filesystem::path{});
    tempo_config::SeqTParser extraComponentDirsParser(&pathParser, {});
    tempo_config::BooleanParser ignoreExistingParser(false);
    tempo_config::PathParser projectPathParser;

    std::vector<tempo_command::Default> defaults = {
        {"projectConfigFile", "Use specified project.config file", "FILE"},
        {"copyTargetList", "Copy existing target", "PATH"},
        {"copyConfigList", "Copy existing config", "PATH"},
        {"copyTargetsDirectory", "Copy all targets from existing directory", "DIR"},
        {"copyConfigDirectory", "Copy configs from existing directory", "DIR"},
        {"extraComponentDirs", "Additional component directories", "DIR"},
        {"ignoreExisting", "Do nothing if project already exists"},
        {"projectPath", "Path to the new project", "PATH"},
    };

    const std::vector<tempo_command::Grouping> groupings = {
        {"projectConfigFile", {"--project-config-file"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"copyTargetList", {"--copy-target"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"copyConfigList", {"--copy-config"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"copyTargetsDirectory", {"--copy-targets-dir"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"copyConfigDirectory", {"--copy-config-dir"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"extraComponentDirs", {"--extra-distribution"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"ignoreExisting", {"--ignore-existing"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"help", {"-h", "--help"}, tempo_command::GroupingType::HELP_FLAG},
    };

    const std::vector<tempo_command::Mapping> optMappings = {
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "projectConfigFile"},
        {tempo_command::MappingType::ANY_INSTANCES, "copyTargetList"},
        {tempo_command::MappingType::ANY_INSTANCES, "copyConfigList"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "copyTargetsDirectory"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "copyConfigDirectory"},
        {tempo_command::MappingType::TRUE_IF_INSTANCE, "ignoreExisting"},
        {tempo_command::MappingType::ANY_INSTANCES, "extraComponentDirs"},
    };

    std::vector<tempo_command::Mapping> argMappings = {
        {tempo_command::MappingType::ONE_INSTANCE, "projectPath"},
    };

    tempo_command::OptionsHash options;
    tempo_command::ArgumentVector arguments;

    // parse global options and arguments
    auto status = tempo_command::parse_completely(tokens, groupings, options, arguments);
    if (status.notOk()) {
        tempo_command::CommandStatus commandStatus;
        if (!status.convertTo(commandStatus))
            return status;
        switch (commandStatus.getCondition()) {
            case tempo_command::CommandCondition::kHelpRequested:
                tempo_command::display_help_and_exit({"zuri-project", "new"},
                    "Create a new project",
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

    TU_LOG_V << "new config:\n" << tempo_command::command_config_to_string(commandConfig);

    // construct command map
    tempo_config::ConfigMap commandMap(commandConfig);

    // determine project config file
    std::filesystem::path projectConfigFile;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(projectConfigFile, projectConfigFileParser,
        commandConfig, "projectConfigFile"));

    // determine list of targets to copy from
    std::vector<std::filesystem::path> copyTargetList;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(copyTargetList, copyTargetListParser,
        commandConfig, "copyTargetList"));

    // determine list of targets to copy from
    std::vector<std::filesystem::path> copyConfigList;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(copyConfigList, copyConfigListParser,
        commandConfig, "copyConfigList"));

    // determine targets directory to copy from
    std::filesystem::path copyTargetsDirectory;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(copyTargetsDirectory, copyTargetsDirectoryParser,
        commandConfig, "copyTargetsDirectory"));

    // determine config directory to copy from
    std::filesystem::path copyConfigDirectory;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(copyConfigDirectory, copyConfigDirectoryParser,
        commandConfig, "copyConfigDirectory"));

    // determine extra component directories
    std::vector<std::filesystem::path> extraComponentDirs;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(extraComponentDirs, extraComponentDirsParser,
        commandConfig, "extraComponentDirs"));

    // determine whether to ignore existing project
    bool ignoreExisting;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(ignoreExisting, ignoreExistingParser,
        commandConfig, "ignoreExisting"));

    // determine the path to the new project directory
    std::filesystem::path projectPath;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(projectPath, projectPathParser,
        commandConfig, "projectPath"));

    if (std::filesystem::exists(projectPath)) {
        if (ignoreExisting) {
            zuri_tooling::Project existingProject;
            TU_ASSIGN_OR_RETURN (existingProject, zuri_tooling::Project::open(projectPath));
            return {};
        }
        return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
            "directory '{}' already exists", projectPath.string());
    }

    zuri_tooling::ProjectOpenOrCreateOptions openOrCreateOptions;
    openOrCreateOptions.exclusive = true;

    // parse the project config file if specified
    if (!projectConfigFile.empty()) {
        TU_ASSIGN_OR_RETURN (openOrCreateOptions.projectMap, tempo_config::read_config_map_file(projectConfigFile));
        TU_LOG_V << "using project config file " << projectConfigFile;
    }

    // create project directory structure
    zuri_tooling::Project project;
    TU_ASSIGN_OR_RETURN (project, zuri_tooling::Project::openOrCreate(projectPath, openOrCreateOptions));
    TU_LOG_V << "creating project " << projectPath;

    using copyopts = std::filesystem::copy_options;
    auto copyTargetOptions = copyopts::recursive;
    auto copyConfigOptions = copyopts::recursive;
    std::error_code ec;

    // if --copy-targets-dir was specified then add all directory entries to copyTargetList
    if (!copyTargetsDirectory.empty()) {
        if (!std::filesystem::is_directory(copyTargetsDirectory))
            return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
                "failed to copy targets directory; '{}' does not exist", copyTargetsDirectory.string());
        std::filesystem::directory_iterator targetsIterator(copyTargetsDirectory, ec);
        if (ec)
            return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
                "failed to list directory {}; {}", copyTargetsDirectory.string(), ec.message());
        for (const auto &entry : targetsIterator) {
            copyTargetList.push_back(entry.path());
        }
    }

    // copy targets into the project
    for (const auto &copyTarget : copyTargetList) {
        if (!std::filesystem::is_directory(copyTarget))
            return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
                "failed to copy target; directory '{}' does not exist", copyTarget.string());
        auto absoluteCopyTarget = std::filesystem::absolute(copyTarget);
        auto destinationDirectory = project.getTargetsDirectory() / copyTarget.filename();
        TU_LOG_V << "copying target from " << absoluteCopyTarget << " to " << destinationDirectory;
        std::filesystem::copy(absoluteCopyTarget, destinationDirectory, copyTargetOptions, ec);
        if (ec)
            return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
                "failed to copy '{}' target from {}; {}",
                copyTarget.filename().string(), absoluteCopyTarget.string(), ec.message());
    }

    // if --copy-config-dir was specified then add all directory entries to copyConfigList
    if (!copyConfigDirectory.empty()) {
        if (!std::filesystem::is_directory(copyConfigDirectory))
            return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
                "failed to copy config directory; '{}' does not exist", copyConfigDirectory.string());
        std::filesystem::directory_iterator configIterator(copyConfigDirectory, ec);
        if (ec)
            return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
                "failed to list directory {}; {}", copyConfigDirectory.string(), ec.message());
        for (const auto &entry : configIterator) {
            copyConfigList.push_back(entry.path());
        }
    }

    // copy configs into the project
    for (const auto &copyConfig : copyConfigList) {
        if (!std::filesystem::exists(copyConfig))
            return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
                "failed to copy config; '{}' does not exist", copyConfig.string());
        auto absoluteCopyConfig = std::filesystem::absolute(copyConfig);
        TU_LOG_V << "copying configs from " << absoluteCopyConfig << " to " << project.getConfigDirectory();
        std::filesystem::copy(absoluteCopyConfig, project.getConfigDirectory(), copyConfigOptions, ec);
        if (ec)
            return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
                "failed to copy '{}' config from {}; {}",
                copyConfig.filename().string(), absoluteCopyConfig.string(), ec.message());
    }

    return {};
}
