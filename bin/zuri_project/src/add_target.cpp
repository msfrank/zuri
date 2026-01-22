
#include <absl/strings/match.h>
#include <mustache/mustache.hpp>

#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/enum_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_utils/file_reader.h>
#include <tempo_utils/file_writer.h>
#include <zuri_packager/package_specifier.h>
#include <zuri_project/add_target.h>
#include <zuri_project/project_result.h>
#include <zuri_project/template_config.h>

using namespace kainjow;

static tempo_utils::Result<std::string>
process_template(const std::filesystem::path &templateFile, mustache::data &templateArguments)
{
    tempo_utils::FileReader templateReader(templateFile);
    TU_RETURN_IF_NOT_OK (templateReader.getStatus());
    auto bytes = templateReader.getBytes();
    std::string templateString(bytes->getStringView());

    mustache::mustache mustacheTemplate(templateString);
    if (!mustacheTemplate.is_valid())
        return zuri_project::ProjectStatus::forCondition(
            zuri_project::ProjectCondition::kProjectInvariant,
            "template contains error; {}", mustacheTemplate.error_message());

    auto templateOutput = mustacheTemplate.render(templateArguments);
    return templateOutput;
}

tempo_utils::Result<tempo_config::ConfigMap>
zuri_project::add_target(
    std::shared_ptr<TemplateConfig> templateConfig,
    std::string_view name,
    const zuri_packager::PackageSpecifier &specifier,
    const absl::flat_hash_map<std::string,std::string> &userArguments,
    const std::filesystem::path &targetsDirectory)
{
    auto tmpl = templateConfig->getTemplate();
    auto targetConfigTemplateFile = tmpl.getTargetConfigTemplateFile();
    mustache::data templateArguments;

    // target arguments
    templateArguments.set("target::name", std::string(name));
    templateArguments.set("target::specifier", specifier.toString());
    templateArguments.set("target::packageName", specifier.getPackageName());
    templateArguments.set("target::packageDomain", specifier.getPackageDomain());
    templateArguments.set("target::packageVersion", specifier.getPackageVersion().toString());
    templateArguments.set("target::majorVersion", specifier.getMajorVersion());
    templateArguments.set("target::minorVersion", specifier.getMinorVersion());
    templateArguments.set("target::patchVersion", specifier.getPatchVersion());
    templateArguments.set("target::createdAt", absl::FormatTime(absl::Now()));

    // user arguments
    for (const auto &entry : userArguments) {
        const auto &parameterName = entry.first;
        if (absl::StrContains(parameterName, "::"))
            return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
                "invalid template argument name '{}'; user argument name cannot contain '::'",
                parameterName);
        if (!templateConfig->hasParameter(parameterName))
            return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
                "invalid user argument '{}'; template parameter not defined",
                parameterName);
        templateArguments.set(parameterName, entry.second);
    }

    // process the template
    std::string templateOutput;
    TU_ASSIGN_OR_RETURN (templateOutput, process_template(targetConfigTemplateFile, templateArguments));

    // parse the output as a config map
    tempo_config::ConfigNode outputNode;
    TU_ASSIGN_OR_RETURN (outputNode, tempo_config::read_config_string(templateOutput));
    if (outputNode.getNodeType() != tempo_config::ConfigNodeType::kMap)
        return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
            "invalid template output; target config must be a map");

    // verify target does not exist
    auto targetDirectory = targetsDirectory / name;
    if (std::filesystem::exists(targetDirectory))
        return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
            "target '{}' already exists at {}", name, targetDirectory.string());

    // create the target directory
    std::error_code ec;
    std::filesystem::create_directory(targetDirectory, ec);
    if (ec)
        return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
            "failed to create target directory {}", targetDirectory.string());

    // iterate recursively over each entry in the content root
    auto contentRoot = templateConfig->getContentRoot();
    std::filesystem::recursive_directory_iterator contentIt(contentRoot);
    if (ec)
        return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
            "failed to process content root {}; {}", contentRoot.string());

    // copy all content to the target directory
    for (const auto &entry : contentIt) {
        const auto &sourcePath = entry.path();
        auto sourcePerms = entry.status().permissions();
        auto relativePath = entry.path().lexically_relative(contentRoot);
        auto destinationPath = targetDirectory / relativePath;

        if (entry.is_regular_file()) {
            auto filename = sourcePath.filename();
            if (filename.extension() == ".mustache") {
                destinationPath.replace_extension();
                TU_LOG_V << "processing template at " << relativePath << " and write to " << destinationPath;
                std::string renderedFile;
                TU_ASSIGN_OR_RETURN (renderedFile, process_template(sourcePath, templateArguments));
                tempo_utils::FileWriter fileWriter(destinationPath, renderedFile,
                    tempo_utils::FileWriterMode::CREATE_ONLY, sourcePerms);
                TU_RETURN_IF_NOT_OK (fileWriter.getStatus());
            } else {
                TU_LOG_V << "copying file from " << relativePath << " to " << destinationPath;
                std::filesystem::copy_file(sourcePath, destinationPath, ec);
                if (ec)
                    return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
                        "failed to copy file {}; {}", sourcePath.string(), ec.message());
                std::filesystem::permissions(destinationPath, sourcePerms, ec);
                if (ec)
                    return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
                        "failed to set permissions on {}; {}", destinationPath.string(), ec.message());
            }
        } else if (entry.is_directory()) {
            TU_LOG_V << "creating directory " << relativePath << " at " << destinationPath;
            std::filesystem::create_directory(destinationPath, ec);
            if (ec)
                return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
                    "failed to create directory {}; {}", destinationPath.string(), ec.message());
            std::filesystem::permissions(destinationPath, sourcePerms, ec);
            if (ec)
                return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
                    "failed to set permissions on {}; {}", destinationPath.string(), ec.message());
        } else {
            return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
                "invalid template content at {}", entry.path().string());
        }
    }

    return outputNode.toMap();
}

tempo_utils::Status
zuri_project::validate_user_arguments(
    std::shared_ptr<TemplateConfig> templateConfig,
    absl::flat_hash_map<std::string,std::string> &userArguments,
    bool interactive)
{
    for (auto it = templateConfig->parametersBegin(); it != templateConfig->parametersEnd(); ++it) {
        const auto &paramName = it->first;
        const auto &paramEntry = it->second;
        auto entry = userArguments.find(paramName);
        if (entry == userArguments.cend()) {
            if (interactive) {
                TU_CONSOLE_OUT << "prompt for argument value";
                continue;
            }
            if (paramEntry->optional) {
                continue;
            }
            return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
                "missing template argument for '{}'", paramName);
        }
    }
    return {};
}