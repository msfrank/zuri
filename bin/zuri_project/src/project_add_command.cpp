/* SPDX-License-Identifier: AGPL-3.0-or-later */

#include <tempo_command/command.h>
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
    PairKVParser templateStringArgumentParser(&nameParser, &valueParser);
    tempo_config::SeqTParser templateStringArgumentListParser(&templateStringArgumentParser, {});
    tempo_config::ConfigStringParser configStringParser;
    PairKVParser templateJsonArgumentParser(&nameParser, &configStringParser);
    tempo_config::SeqTParser templateJsonArgumentListParser(&templateJsonArgumentParser, {});
    tempo_config::StringParser packageNameParser(std::string{});
    zuri_packager::PackageVersionParser packageVersionParser(zuri_packager::PackageVersion(0, 0, 1));
    tempo_config::StringParser packageDomainParser("localhost");
    tempo_config::StringParser targetNameParser;

    tempo_command::Command command(std::vector<std::string>{"zuri-project", "add"});

    command.addArgument("targetName", "TARGET", tempo_command::MappingType::ONE_INSTANCE,
        "Name of the target");
    command.addOption("projectRoot", {"-P", "--project-root"}, tempo_command::MappingType::ZERO_OR_ONE_INSTANCE,
        "Specify an alternative project root directory", "DIR");
    command.addOption("templatesRoot", {"-T", "--templates-root"}, tempo_command::MappingType::ZERO_OR_ONE_INSTANCE,
        "Specify an alternative templates directory", "DIR");
    command.addOption("targetTemplate", {"-t", "--target-template"}, tempo_command::MappingType::ZERO_OR_ONE_INSTANCE,
        "Create target using the specified template", "TEMPLATE");
    command.addOption("templateStringArgumentList", {"-s", "--string-argument"}, tempo_command::MappingType::ANY_INSTANCES,
        "Specify template argument string value", "NAME:VALUE");
    command.addOption("templateJsonArgumentList", {"-j", "--json-argument"}, tempo_command::MappingType::ANY_INSTANCES,
        "Specify template argument JSON value", "NAME:JSON");
    command.addOption("packageName", {"--package-name"}, tempo_command::MappingType::ZERO_OR_ONE_INSTANCE,
        "The target package name", "NAME");
    command.addOption("packageVersion", {"--package-version"}, tempo_command::MappingType::ZERO_OR_ONE_INSTANCE,
        "The target package version", "VERSION");
    command.addOption("packageDomain", {"--package-domain"}, tempo_command::MappingType::ZERO_OR_ONE_INSTANCE,
        "The target package domain", "DOMAIN");
    command.addHelpOption("help", {"-h", "--help"}, "Add a new target to the project");

    TU_RETURN_IF_NOT_OK (command.parseCompletely(tokens));

    // determine the project root
    std::filesystem::path projectRoot;
    TU_RETURN_IF_NOT_OK(command.convert(projectRoot, projectRootParser, "projectRoot"));

    // determine the templates root
    std::filesystem::path templatesRoot;
    TU_RETURN_IF_NOT_OK (command.convert(templatesRoot, templatesRootParser, "templatesRoot"));

    // determine template to use when creating target
    std::string targetTemplate;
    TU_RETURN_IF_NOT_OK (command.convert(targetTemplate, targetTemplateParser, "targetTemplate"));

    // determine list of string arguments to make available when processing templates
    std::vector<std::pair<std::string,std::string>> templateStringArgumentList;
    TU_RETURN_IF_NOT_OK (command.convert(templateStringArgumentList, templateStringArgumentListParser, "templateStringArgumentList"));

    // determine list of template arguments to make available when processing templates
    std::vector<std::pair<std::string,tempo_config::ConfigNode>> templateJsonArgumentList;
    TU_RETURN_IF_NOT_OK (command.convert(templateJsonArgumentList, templateJsonArgumentListParser, "templateJsonArgumentList"));

    // determine the name of the target to add
    std::string targetName;
    TU_RETURN_IF_NOT_OK (command.convert(targetName, targetNameParser, "targetName"));

    // determine the package name
    std::string packageName;
    TU_RETURN_IF_NOT_OK (command.convert(packageName, packageNameParser, "packageName"));
    if (packageName.empty()) {
        packageName = targetName;
    }

    // determine the package version
    zuri_packager::PackageVersion packageVersion;
    TU_RETURN_IF_NOT_OK (command.convert(packageVersion, packageVersionParser, "packageVersion"));

    // determine the package domain
    std::string packageDomain;
    TU_RETURN_IF_NOT_OK (command.convert(packageDomain, packageDomainParser, "packageDomain"));

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

    // create new target from the specified template
    zuri_packager::PackageId packageId(packageName, packageDomain);
    zuri_packager::PackageSpecifier specifier(packageId, packageVersion);
    tempo_config::ConfigMap targetMap;
    TU_ASSIGN_OR_RETURN (targetMap, add_target(templateConfig, targetName, specifier,
        templateStringArgumentList, templateJsonArgumentList, project.getTargetsDirectory()));

    // load the project.config for editing
    tempo_config::ConfigFileEditor projectConfigEditor(project.getProjectConfigFile());
    TU_RETURN_IF_NOT_OK (projectConfigEditor.getStatus());

    // add the target config to the project
    tempo_config::ConfigPath root;
    auto targetsPath = root.traverse("targets");
    if (!projectConfigEditor.hasNode(targetsPath)) {
        TU_RETURN_IF_NOT_OK (projectConfigEditor.insertNode(targetsPath, tempo_config::ConfigMap{}));
    }

    // write the project.config to disk
    tempo_config::PrinterOptions printerOptions;
    TU_RETURN_IF_NOT_OK (projectConfigEditor.insertNode(targetsPath.traverse(targetName), targetMap));
    TU_RETURN_IF_NOT_OK (projectConfigEditor.writeFile(printerOptions));

    return {};
}
