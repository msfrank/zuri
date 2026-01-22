#ifndef ZURI_PROJECT_ADD_TARGET_H
#define ZURI_PROJECT_ADD_TARGET_H

#include <filesystem>

#include <tempo_config/config_types.h>
#include <tempo_utils/result.h>
#include <zuri_packager/package_specifier.h>

#include "template_config.h"

namespace zuri_project {

    tempo_utils::Status validate_user_arguments(
        std::shared_ptr<TemplateConfig> templateConfig,
        absl::flat_hash_map<std::string,std::string> &userArguments,
        bool interactive);

    tempo_utils::Result<tempo_config::ConfigMap> add_target(
        std::shared_ptr<TemplateConfig> templateConfig,
        std::string_view name,
        const zuri_packager::PackageSpecifier &specifier,
        const absl::flat_hash_map<std::string,std::string> &userArguments,
        const std::filesystem::path &targetsDirectory);
}

#endif // ZURI_PROJECT_ADD_TARGET_H