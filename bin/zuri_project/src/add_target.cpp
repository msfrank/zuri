
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
#include <zuri_project/template_processor.h>

tempo_utils::Status
zuri_project::add_target(
    const AddTargetOperation &op,
    std::shared_ptr<TemplateConfig> templateConfig,
    const std::filesystem::path &targetsDirectory,
    tempo_config::ConfigFileEditor &projectConfigEditor)
{
    TU_ASSERT (!op.name.empty());
    TU_ASSERT (op.specifier.isValid());
    TU_ASSERT (templateConfig != nullptr);

    auto tmpl = templateConfig->getTemplate();
    auto targetConfigTemplateFile = tmpl.getTargetConfigTemplateFile();

    TemplateProcessor templateProcessor(templateConfig);

    // target metadata
    templateProcessor.putMetadata("target", "name", std::string(op.name));
    templateProcessor.putMetadata("target", "specifier", op.specifier.toString());
    templateProcessor.putMetadata("target", "packageName", op.specifier.getPackageName());
    templateProcessor.putMetadata("target", "packageDomain", op.specifier.getPackageDomain());
    templateProcessor.putMetadata("target", "packageVersion", op.specifier.getPackageVersion().toString());
    templateProcessor.putMetadata("target", "majorVersion", absl::StrCat(op.specifier.getMajorVersion()));
    templateProcessor.putMetadata("target", "minorVersion", absl::StrCat(op.specifier.getMinorVersion()));
    templateProcessor.putMetadata("target", "patchVersion", absl::StrCat(op.specifier.getPatchVersion()));
    templateProcessor.putMetadata("target", "createdAt", absl::FormatTime(absl::Now()));

    // user arguments
    for (const auto &p : op.stringArguments) {
        templateProcessor.putArgument(p.first, p.second);
    }
    for (const auto &p : op.jsonArguments) {
        templateProcessor.putArgument(p.first, p.second);
    }

    // ensure that all required template arguments are present
    //TU_RETURN_IF_NOT_OK (validate_user_arguments(templateConfig, templateArguments, false));

    // process the template
    std::string templateOutput;
    TU_ASSIGN_OR_RETURN (templateOutput, templateProcessor.processTemplate(targetConfigTemplateFile));

    // parse the output as a config map
    tempo_config::ConfigNode outputNode;
    TU_ASSIGN_OR_RETURN (outputNode, tempo_config::read_config_string(templateOutput));
    if (outputNode.getNodeType() != tempo_config::ConfigNodeType::kMap)
        return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
            "invalid template output; target config must be a map");

    // verify target does not exist
    auto targetDirectory = targetsDirectory / op.name;
    if (std::filesystem::exists(targetDirectory))
        return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
            "target '{}' already exists at {}", op.name, targetDirectory.string());

    // create the target directory
    std::error_code ec;
    std::filesystem::create_directory(targetDirectory, ec);
    if (ec)
        return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
            "failed to create target directory {}", targetDirectory.string());

    // iterate recursively over each entry in the content root
    auto contentRoot = templateConfig->getContentRoot();
    std::filesystem::recursive_directory_iterator contentIt(contentRoot, ec);
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
                TU_ASSIGN_OR_RETURN (renderedFile, templateProcessor.processTemplate(sourcePath));
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

    auto targetMap = outputNode.toMap();

    tempo_config::ConfigPath root;

    // if the targets node is not present in the project config then add it
    auto targetsPath = root.traverse("targets");
    if (!projectConfigEditor.hasNode(targetsPath)) {
        TU_RETURN_IF_NOT_OK (projectConfigEditor.insertNode(targetsPath, tempo_config::ConfigMap{}));
    }

    // add the target config to the project
    auto targetEntryPath = targetsPath.traverse(op.name);
    TU_RETURN_IF_NOT_OK (projectConfigEditor.insertNode(targetEntryPath, targetMap));

    return {};
}