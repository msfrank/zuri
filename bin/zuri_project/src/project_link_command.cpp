/* SPDX-License-Identifier: AGPL-3.0-or-later */

#include <tempo_command/command.h>
#include <tempo_config/abstract_converter.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/enum_conversions.h>
#include <tempo_utils/result.h>
#include <zuri_distributor/dependency_selector.h>
#include <zuri_distributor/package_fetcher.h>
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

    tempo_command::Command command(std::vector<std::string>{"zuri-project", "link"});

    command.addArgument("targetPath", "TARGET", tempo_command::MappingType::ONE_INSTANCE,
        "The target project to link to");
    command.addArgument("projectPath", "PATH", tempo_command::MappingType::ONE_INSTANCE,
        "Path to the new project");
    command.addOption("linkProjectConfigFile", {"--link-project-config-file"}, tempo_command::MappingType::ZERO_OR_ONE_INSTANCE,
        "Link the specified project.config file", "FILE");
    command.addOption("extraLibDirList", {"--extra-lib-dir"}, tempo_command::MappingType::ANY_INSTANCES,
        "Additional component directories", "DIR");
    command.addOption("ifExisting", {"--if-existing"}, tempo_command::MappingType::ZERO_OR_ONE_INSTANCE,
        "Specify behavior if project already exists", "ENUM");
    command.addOption("recreateIfFilePresent", {"--recreate-if-file-present"}, tempo_command::MappingType::ZERO_OR_ONE_INSTANCE,
        "Recreate project if specified file is present", "PATH");
    command.addHelpOption("help", {"-h", "--help"}, "Create a linked Zuri project");

    TU_RETURN_IF_NOT_OK (command.parseCompletely(tokens));

    // determine project config file  to link from
    std::filesystem::path linkProjectConfigFile;
    TU_RETURN_IF_NOT_OK (command.convert(linkProjectConfigFile, linkProjectConfigFileParser, "linkProjectConfigFile"));

    // determine list of extra lib directories
    std::vector<std::filesystem::path> extraLibDirList;
    TU_RETURN_IF_NOT_OK (command.convert(extraLibDirList, extraLibDirListParser, "extraLibDirList"));

    // determine what to do if project already exists
    IfExisting ifExisting;
    TU_RETURN_IF_NOT_OK (command.convert(ifExisting, ifExistingParser, "ifExisting"));

    // determine file to check for existence if Recreate is set
    std::filesystem::path recreateIfFilePresent;
    TU_RETURN_IF_NOT_OK (command.convert(recreateIfFilePresent, recreateIfFilePresentParser, "recreateIfFilePresent"));

    // determine the path to the target project directory
    std::filesystem::path targetPath;
    TU_RETURN_IF_NOT_OK (command.convert(targetPath, targetPathParser, "targetPath"));

    // determine the path to the new project directory
    std::filesystem::path projectPath;
    TU_RETURN_IF_NOT_OK (command.convert(projectPath, projectPathParser, "projectPath"));

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

    TU_LOG_INFO << "linking to existing project at " << targetPath;

    // create project directory structure
    zuri_tooling::Project project;
    TU_ASSIGN_OR_RETURN (project, zuri_tooling::Project::openOrCreate(projectPath, openOrCreateOptions));

    return {};
}
