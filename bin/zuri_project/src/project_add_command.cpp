
#include <tempo_command/command_help.h>
#include <tempo_command/command_parser.h>
#include <tempo_config/abstract_converter.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/config_file_editor.h>
#include <tempo_config/container_conversions.h>
#include <tempo_utils/result.h>
#include <zuri_distributor/dependency_selector.h>
#include <zuri_distributor/package_fetcher.h>
#include <zuri_packager/packaging_conversions.h>
#include <zuri_project/add_target.h>
#include <zuri_project/project_add_command.h>
#include <zuri_project/project_conversions.h>
#include <zuri_project/project_result.h>
#include <zuri_tooling/project.h>
#include <zuri_tooling/project_config.h>

tempo_utils::Status
zuri_project::project_add_command(
    std::shared_ptr<zuri_tooling::CoreConfig> coreConfig,
    tempo_command::TokenVector &tokens)
{
    tempo_config::PathParser projectRootParser(std::filesystem::path{});
    tempo_config::PathParser templatesRootParser(std::filesystem::path{});
    tempo_config::StringParser targetTemplateParser;
    tempo_config::StringParser nameParser;
    tempo_config::StringParser valueParser;
    PairKVParser templateArgumentParser(&nameParser, &valueParser);
    tempo_config::SeqTParser templateArgumentListParser(&templateArgumentParser, {});
    tempo_config::StringParser packageNameParser(std::string{});
    zuri_packager::PackageVersionParser packageVersionParser(zuri_packager::PackageVersion(0, 0, 1));
    tempo_config::StringParser packageDomainParser("localhost");
    tempo_config::StringParser targetNameParser;

    std::vector<tempo_command::Default> defaults = {
        {"projectRoot", "Specify an alternative project root directory", "DIR"},
        {"templatesRoot", "Specify an alternative templates directory", "DIR"},
        {"targetTemplate", "Create target using the specified template", "TEMPLATE"},
        {"templateArgumentList", "Specify template argument", "NAME:VALUE"},
        {"packageName", "The target package name", "NAME"},
        {"packageVersion", "The target package version", "VERSION"},
        {"packageDomain", "The target package domain", "DOMAIN"},
        {"targetName", "Name of the target", "TARGET-NAME"},
    };

    const std::vector<tempo_command::Grouping> groupings = {
        {"projectRoot", {"-P", "--project-root"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"projectRoot", {"-T", "--templates-root"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"targetTemplate", {"-t", "--target-template"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"templateArgumentList", {"-a", "--template-argument"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"packageName", {"--package-name"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"packageVersion", {"--package-version"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"packageDomain", {"--package-domain"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"help", {"-h", "--help"}, tempo_command::GroupingType::HELP_FLAG},
    };

    const std::vector<tempo_command::Mapping> optMappings = {
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "projectRoot"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "templatesRoot"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "targetTemplate"},
        {tempo_command::MappingType::ANY_INSTANCES, "templateArgumentList"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "packageName"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "packageVersion"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "packageDomain"},
    };

    std::vector<tempo_command::Mapping> argMappings = {
        {tempo_command::MappingType::ONE_INSTANCE, "targetName"},
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

    // determine the project root
    std::filesystem::path projectRoot;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(projectRoot, projectRootParser,
        commandConfig, "projectRoot"));

    // determine the templates root
    std::filesystem::path templatesRoot;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(templatesRoot, templatesRootParser,
        commandConfig, "templatesRoot"));

    // determine template to use when creating target
    std::string targetTemplate;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(targetTemplate, targetTemplateParser,
        commandConfig, "targetTemplate"));

    // determine list of template arguments to make available when processing templates
    std::vector<std::pair<std::string,std::string>> templateArgumentList;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(templateArgumentList, templateArgumentListParser,
        commandConfig, "templateArgumentList"));

    // determine the name of the target to add
    std::string targetName;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(targetName, targetNameParser,
        commandConfig, "targetName"));

    // construct map of template arguments
    absl::flat_hash_map<std::string,std::string> templateArguments;
    for (const auto &kv : templateArgumentList) {
        auto curr = templateArguments.find(kv.first);
        if (curr != templateArguments.cend())
            tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kInvalidConfiguration,
                "--template-argument '{}' already specified; current value is '{}'",
                curr->first, curr->second);
        templateArguments[kv.first] = kv.second;
    }

    // determine the package name
    std::string packageName;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(packageName, packageNameParser,
        commandConfig, "packageName"));
    if (packageName.empty()) {
        packageName = targetName;
    }

    // determine the package version
    zuri_packager::PackageVersion packageVersion;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(packageVersion, packageVersionParser,
        commandConfig, "packageVersion"));

    // determine the package domain
    std::string packageDomain;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(packageDomain, packageDomainParser,
        commandConfig, "packageDomain"));

    // open the project
    zuri_tooling::Project project;
    if (!projectRoot.empty()) {
        TU_ASSIGN_OR_RETURN (project, zuri_tooling::Project::open(projectRoot));
    } else {
        TU_ASSIGN_OR_RETURN (project, zuri_tooling::Project::find(std::filesystem::current_path()));
    }

    // linked projects cannot be modified
    if (project.isLinked())
        tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kInvalidConfiguration,
            "cannot add a target to a linked project");

    // load the project config
    std::shared_ptr<zuri_tooling::ProjectConfig> projectConfig;
    TU_ASSIGN_OR_RETURN (projectConfig, zuri_tooling::ProjectConfig::load(project, coreConfig));

    // verify the target entry does not already exist in the target store
    auto targetStore = projectConfig->getTargetStore();
    if (targetStore->hasTarget(targetName))
        return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
            "target '{}' already exists in project.config", targetName);

    // verify the target directory does not already exist
    auto targetPath = project.getTargetsDirectory() / targetName;
    if (std::filesystem::exists(targetPath))
        return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
            "target directory '{}' already exists", targetPath.string());

    // open the template
    auto templatePath = templatesRoot / targetTemplate;
    Template tmpl;
    TU_ASSIGN_OR_RETURN (tmpl, Template::open(templatePath));

    // read the template config
    std::shared_ptr<TemplateConfig> templateConfig;
    TU_ASSIGN_OR_RETURN (templateConfig, TemplateConfig::load(tmpl));

    // ensure that all required template arguments are present
    TU_RETURN_IF_NOT_OK (validate_user_arguments(templateConfig, templateArguments, false));

    // create new target from the specified template
    zuri_packager::PackageId packageId(packageName, packageDomain);
    zuri_packager::PackageSpecifier specifier(packageId, packageVersion);
    tempo_config::ConfigMap targetMap;
    TU_ASSIGN_OR_RETURN (targetMap, add_target(
        templateConfig, targetName, specifier, templateArguments, project.getTargetsDirectory()));

    // add the target config to the project
    tempo_config::ConfigFileEditor projectConfigEditor(project.getProjectConfigFile());
    TU_RETURN_IF_NOT_OK (projectConfigEditor.getStatus());

    tempo_config::ConfigPath root;
    auto targetsPath = root.traverse("targets");
    if (!projectConfigEditor.hasNode(targetsPath)) {
        TU_RETURN_IF_NOT_OK (projectConfigEditor.insertNode(targetsPath, tempo_config::ConfigMap{}));
    }

    tempo_config::PrinterOptions printerOptions;
    TU_RETURN_IF_NOT_OK (projectConfigEditor.insertNode(targetsPath.traverse(targetName), targetMap));
    TU_RETURN_IF_NOT_OK (projectConfigEditor.writeFile(printerOptions));

    return {};
}
