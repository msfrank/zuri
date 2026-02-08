#ifndef ZURI_PROJECT_ADD_TARGET_H
#define ZURI_PROJECT_ADD_TARGET_H

#include <filesystem>
#include <tempo_config/config_file_editor.h>

#include <tempo_config/config_types.h>
#include <tempo_utils/result.h>
#include <zuri_packager/package_specifier.h>

#include "template_config.h"

namespace zuri_project {

    struct AddTargetOperation {
        std::string name;
        zuri_packager::PackageSpecifier specifier;
        std::vector<std::pair<std::string,std::string>> stringArguments;
        std::vector<std::pair<std::string,tempo_config::ConfigNode>> jsonArguments;
    };

    tempo_utils::Status add_target(
        const AddTargetOperation &op,
        std::shared_ptr<TemplateConfig> templateConfig,
        const std::filesystem::path &targetsDirectory,
        tempo_config::ConfigFileEditor &projectConfigEditor);
}

#endif // ZURI_PROJECT_ADD_TARGET_H