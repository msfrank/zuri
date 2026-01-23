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
#include <zuri_project/project_new_command.h>
#include <zuri_project/project_result.h>
#include <zuri_tooling/project.h>

#include "zuri_project/project_conversions.h"

tempo_utils::Status
zuri_project::project_new_command(
    std::shared_ptr<zuri_tooling::CoreConfig> coreConfig,
    tempo_command::TokenVector &tokens)
{
    tempo_config::PathParser copyProjectConfigFileParser(std::filesystem::path{});
    tempo_config::PathParser linkProjectConfigFileParser(std::filesystem::path{});
    tempo_config::PathParser pathParser;
    tempo_config::SeqTParser copyTargetListParser(&pathParser, {});
    tempo_config::PathParser copyTargetsDirectoryParser(std::filesystem::path{});
    tempo_config::SeqTParser linkTargetListParser(&pathParser, {});
    tempo_config::PathParser linkTargetsDirectoryParser(std::filesystem::path{});
    tempo_config::StringParser nameParser;
    tempo_config::SeqTParser addProgramTargetListParser(&nameParser, {});
    tempo_config::SeqTParser addLibraryTargetListParser(&nameParser, {});
    tempo_config::StringParser valueParser;
    PairKVParser templateArgumentParser(&nameParser, &valueParser);
    tempo_config::SeqTParser templateArgumentListParser(&templateArgumentParser, {});
    tempo_config::SeqTParser extraLibDirListParser(&pathParser, {});
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
        {"copyProjectConfigFile", "Copy the specified project.config file", "FILE"},
        {"linkProjectConfigFile", "Link the specified project.config file", "FILE"},
        {"copyTargetList", "Copy existing target", "PATH"},
        {"copyTargetsDirectory", "Copy all targets from existing directory", "DIR"},
        {"linkTargetList", "Link existing target", "PATH"},
        {"linkTargetsDirectory", "Link all targets from existing directory", "DIR"},
        {"addTemplateTargetList", "Add a new target using the specified template", "TEMPLATE:NAME"},
        {"templateArgumentList", "Specify template argument", "NAME:VALUE"},
        {"extraLibDirList", "Additional component directories", "DIR"},
        {"ifExisting", "Do nothing if project already exists"},
        {"recreateIfFilePresent", "Recreate project if specified file is present", "PATH"},
        {"projectPath", "Path to the new project", "PATH"},
    };

    const std::vector<tempo_command::Grouping> groupings = {
        {"copyProjectConfigFile", {"--copy-project-config-file"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"linkProjectConfigFile", {"--link-project-config-file"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"copyTargetList", {"--copy-target"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"copyTargetsDirectory", {"--copy-targets-dir"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"linkTargetList", {"--link-target"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"linkTargetsDirectory", {"--link-targets-dir"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"addProgramTargetList", {"--add-program-target"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"addLibraryTargetList", {"--add-library-target"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"templateArgumentList", {"-t", "--template-argument"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"extraLibDirList", {"--extra-lib-dir"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"ifExisting", {"--if-existing"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"recreateIfFilePresent", {"--recreate-if-file-present"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"help", {"-h", "--help"}, tempo_command::GroupingType::HELP_FLAG},
    };

    const std::vector<tempo_command::Mapping> optMappings = {
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "copyProjectConfigFile"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "linkProjectConfigFile"},
        {tempo_command::MappingType::ANY_INSTANCES, "copyTargetList"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "copyTargetsDirectory"},
        {tempo_command::MappingType::ANY_INSTANCES, "linkTargetList"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "linkTargetsDirectory"},
        {tempo_command::MappingType::ANY_INSTANCES, "addProgramTargetList"},
        {tempo_command::MappingType::ANY_INSTANCES, "addLibraryTargetList"},
        {tempo_command::MappingType::ANY_INSTANCES, "templateArgumentList"},
        {tempo_command::MappingType::ANY_INSTANCES, "extraLibDirList"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "ifExisting"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "recreateIfFilePresent"},
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

    // determine project config file  to copy from
    std::filesystem::path copyProjectConfigFile;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(copyProjectConfigFile, copyProjectConfigFileParser,
        commandConfig, "copyProjectConfigFile"));

    // determine project config file  to link from
    std::filesystem::path linkProjectConfigFile;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(linkProjectConfigFile, linkProjectConfigFileParser,
        commandConfig, "linkProjectConfigFile"));

    if (!copyProjectConfigFile.empty() && !linkProjectConfigFile.empty())
        return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
            "cannot specify both --copy-project-config-file and --link-project-config-file");

    // determine list of targets to copy from
    std::vector<std::filesystem::path> copyTargetList;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(copyTargetList, copyTargetListParser,
        commandConfig, "copyTargetList"));

    // determine targets directory to copy from
    std::filesystem::path copyTargetsDirectory;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(copyTargetsDirectory, copyTargetsDirectoryParser,
        commandConfig, "copyTargetsDirectory"));

    // determine list of targets to link from
    std::vector<std::filesystem::path> linkTargetList;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(linkTargetList, linkTargetListParser,
        commandConfig, "linkTargetList"));

    // determine targets directory to link from
    std::filesystem::path linkTargetsDirectory;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(linkTargetsDirectory, linkTargetsDirectoryParser,
        commandConfig, "linkTargetsDirectory"));

    // determine list of program targets to add
    std::vector<std::string> addProgramTargetList;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(addProgramTargetList, addProgramTargetListParser,
        commandConfig, "addProgramTargetList"));

    // determine list of library targets to add
    std::vector<std::string> addLibraryTargetList;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(addLibraryTargetList, addLibraryTargetListParser,
        commandConfig, "addLibraryTargetList"));

    // determine list of template arguments to make available when processing templates
    std::vector<std::pair<std::string,std::string>> templateArgumentList;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(templateArgumentList, templateArgumentListParser,
        commandConfig, "templateArgumentList"));

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

    // determine the path to the new project directory
    std::filesystem::path projectPath;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(projectPath, projectPathParser,
        commandConfig, "projectPath"));

    // construct map of template arguments
    absl::flat_hash_map<std::string,std::string> templateArguments;
    for (const auto &kv : templateArgumentList) {
        auto curr = templateArguments.find(kv.first);
        if (curr != templateArguments.cend())
            tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kInvalidConfiguration,
                "--template-argument '{}' already exists; current value is '{}'",
                curr->first, curr->second);
        templateArguments[kv.first] = kv.second;
    }

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

    zuri_tooling::ProjectOpenOrCreateOptions openOrCreateOptions;
    openOrCreateOptions.exclusive = true;
    openOrCreateOptions.distribution = coreConfig->getDistribution();
    openOrCreateOptions.extraLibDirs = extraLibDirList;

    // parse the project config file if specified
    if (!copyProjectConfigFile.empty()) {
        TU_ASSIGN_OR_RETURN (openOrCreateOptions.projectMap, tempo_config::read_config_map_file(copyProjectConfigFile));
        TU_LOG_INFO << "copying project config file " << copyProjectConfigFile;
    } else if (!linkProjectConfigFile.empty()) {
        //openOrCreateOptions.linked = true;
        //openOrCreateOptions.projectConfigTarget = linkProjectConfigFile;
        //TU_LOG_INFO << "linking to project config file " << linkProjectConfigFile;
        TU_UNREACHABLE();
    }

    // create project directory structure
    zuri_tooling::Project project;
    TU_ASSIGN_OR_RETURN (project, zuri_tooling::Project::openOrCreate(projectPath, openOrCreateOptions));

    using copyopts = std::filesystem::copy_options;
    auto copyTargetOptions = copyopts::recursive;
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
        TU_LOG_INFO << "copying target from " << absoluteCopyTarget << " to " << destinationDirectory;
        std::filesystem::copy(absoluteCopyTarget, destinationDirectory, copyTargetOptions, ec);
        if (ec)
            return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
                "failed to copy '{}' target from {}; {}",
                copyTarget.filename().string(), absoluteCopyTarget.string(), ec.message());
    }

    // if --link-targets-dir was specified then add all directory entries to linkTargetList
    if (!linkTargetsDirectory.empty()) {
        if (!std::filesystem::is_directory(linkTargetsDirectory))
            return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
                "failed to link targets directory; '{}' does not exist", linkTargetsDirectory.string());
        std::filesystem::directory_iterator targetsIterator(linkTargetsDirectory, ec);
        if (ec)
            return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
                "failed to list directory {}; {}", linkTargetsDirectory.string(), ec.message());
        for (const auto &entry : targetsIterator) {
            linkTargetList.push_back(entry.path());
        }
    }

    // link targets into the project
    for (const auto &linkTarget : linkTargetList) {
        if (!std::filesystem::is_directory(linkTarget))
            return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
                "failed to link target; directory '{}' does not exist", linkTarget.string());
        auto absoluteLinkTarget = std::filesystem::absolute(linkTarget);
        auto destinationDirectory = project.getTargetsDirectory() / linkTarget.filename();
        TU_LOG_INFO << "linking target from " << absoluteLinkTarget << " to " << destinationDirectory;
        std::filesystem::create_directory_symlink(absoluteLinkTarget, destinationDirectory, ec);
        if (ec)
            return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
                "failed to link '{}' target from {}; {}",
                linkTarget.filename().string(), absoluteLinkTarget.string(), ec.message());
    }

    // add program targets to the project

    // add library targets to the project

    // add template targets to the project

    return {};
}
