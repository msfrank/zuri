/* SPDX-License-Identifier: AGPL-3.0-or-later */

#include <tempo_command/command_help.h>
#include <tempo_command/command_parser.h>
#include <tempo_config/abstract_converter.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/enum_conversions.h>
#include <tempo_utils/result.h>
#include <zuri_distributor/dependency_selector.h>
#include <zuri_distributor/package_fetcher.h>
#include <zuri_project/project_conversions.h>
#include <zuri_project/project_link_command.h>
#include <zuri_project/project_result.h>
#include <zuri_tooling/project.h>

tempo_utils::Status
zuri_project::project_link_command(
    std::shared_ptr<zuri_tooling::CoreConfig> coreConfig,
    tempo_command::TokenVector &tokens)
{
    tempo_config::PathParser linkProjectConfigFileParser(std::filesystem::path{});
    tempo_config::PathParser pathParser;
    tempo_config::SeqTParser extraLibDirListParser(&pathParser, {});
    tempo_config::PathParser targetPathParser;
    tempo_config::PathParser projectPathParser;

    enum IfExisting {
        Abort,
        Skip,
        Recreate,
    };
    tempo_config::EnumTParser<IfExisting> ifExistingParser({
        {"Abort", Abort},
        {"Skip", Skip},
        {"Recreate", Recreate},
    }, Abort);

    tempo_config::PathParser recreateIfFilePresentParser(std::filesystem::path{});

    std::vector<tempo_command::Default> defaults = {
        {"linkProjectConfigFile", "Link the specified project.config file", "FILE"},
        {"extraLibDirList", "Additional component directories", "DIR"},
        {"ifExisting", "Do nothing if project already exists"},
        {"recreateIfFilePresent", "Recreate project if specified file is present", "PATH"},
        {"targetPath", "The target project to link to", "TARGET"},
        {"projectPath", "Path to the new project", "PATH"},
    };

    const std::vector<tempo_command::Grouping> groupings = {
        {"linkProjectConfigFile", {"--link-project-config-file"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"extraLibDirList", {"--extra-lib-dir"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"ifExisting", {"--if-existing"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"recreateIfFilePresent", {"--recreate-if-file-present"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"help", {"-h", "--help"}, tempo_command::GroupingType::HELP_FLAG},
    };

    const std::vector<tempo_command::Mapping> optMappings = {
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "linkProjectConfigFile"},
        {tempo_command::MappingType::ANY_INSTANCES, "extraLibDirList"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "ifExisting"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "recreateIfFilePresent"},
    };

    std::vector<tempo_command::Mapping> argMappings = {
        {tempo_command::MappingType::ONE_INSTANCE, "targetPath"},
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
                tempo_command::display_help_and_exit({"zuri-project", "link"},
                    "Create a linked project",
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

    TU_LOG_V << "link config:\n" << tempo_command::command_config_to_string(commandConfig);

    // construct command map
    tempo_config::ConfigMap commandMap(commandConfig);

    // determine project config file  to link from
    std::filesystem::path linkProjectConfigFile;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(linkProjectConfigFile, linkProjectConfigFileParser,
        commandConfig, "linkProjectConfigFile"));

    // determine list of extra lib directories
    std::vector<std::filesystem::path> extraLibDirList;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(extraLibDirList, extraLibDirListParser,
        commandConfig, "extraLibDirList"));

    // determine what to do if project already exists
    IfExisting ifExisting;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(ifExisting, ifExistingParser,
        commandConfig, "ifExisting"));

    // determine file to check for existence if Recreate is set
    std::filesystem::path recreateIfFilePresent;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(recreateIfFilePresent, recreateIfFilePresentParser,
        commandConfig, "recreateIfFilePresent"));

    // determine the path to the target project directory
    std::filesystem::path targetPath;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(targetPath, targetPathParser,
        commandConfig, "targetPath"));

    // determine the path to the new project directory
    std::filesystem::path projectPath;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(projectPath, projectPathParser,
        commandConfig, "projectPath"));

    if (std::filesystem::exists(projectPath)) {
        switch (ifExisting) {
            // if project directory exists then skip creation
            case Skip: {
                zuri_tooling::Project existingProject;
                TU_ASSIGN_OR_RETURN (existingProject, zuri_tooling::Project::open(projectPath));
                TU_CONSOLE_OUT << "skipping project creation; project already exists";
                return {};
            }
            // if project directory exists then recreate if condition is met or skip creation
            case Recreate: {
                bool recreate = false;
                if (!recreateIfFilePresent.empty()) {
                    recreate = std::filesystem::exists(recreateIfFilePresent);
                }
                if (!recreate) {
                    zuri_tooling::Project existingProject;
                    TU_ASSIGN_OR_RETURN (existingProject, zuri_tooling::Project::open(projectPath));
                    TU_CONSOLE_OUT << "skipping project creation; project already exists";
                    return {};
                }
                break;
            }
            // if project directory exists then abort creation
            case Abort:
            default:
                return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
                    "directory '{}' already exists", projectPath.string());
        }
        std::error_code ec;
        std::filesystem::remove_all(projectPath, ec);
        if (ec)
            return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
                "failed to remove existing project directory '{}'; {}",
                projectPath.string(), ec.message());
        TU_CONSOLE_OUT << "recreating project " << projectPath;
    } else {
        TU_CONSOLE_OUT << "creating project " << projectPath;
    }

    //
    zuri_tooling::ProjectOpenOrCreateOptions openOrCreateOptions;
    openOrCreateOptions.linkedProject = targetPath;
    openOrCreateOptions.linkedProjectConfigFile = linkProjectConfigFile;
    openOrCreateOptions.exclusive = true;
    openOrCreateOptions.distribution = coreConfig->getDistribution();
    openOrCreateOptions.extraLibDirs = extraLibDirList;

    TU_LOG_INFO << "linking to project " << targetPath;

    // create project directory structure
    zuri_tooling::Project project;
    TU_ASSIGN_OR_RETURN (project, zuri_tooling::Project::openOrCreate(projectPath, openOrCreateOptions));

    return {};
}
