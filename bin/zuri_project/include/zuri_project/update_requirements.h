#ifndef ZURI_PROJECT_UPDATE_REQUIREMENTS_H
#define ZURI_PROJECT_UPDATE_REQUIREMENTS_H

#include <tempo_config/config_file_editor.h>
#include <zuri_packager/package_specifier.h>
#include <zuri_tooling/import_store.h>

#include "template_config.h"

namespace zuri_project {

    struct UpdateRequirementsOperation {
        std::vector<zuri_packager::PackageSpecifier> addRequirements;
        std::vector<zuri_packager::PackageId> removeRequirements;
    };

    tempo_utils::Status update_requirements(
        const UpdateRequirementsOperation &op,
        std::shared_ptr<zuri_tooling::ImportStore> importStore,
        tempo_config::ConfigFileEditor &projectConfigEditor);

}

#endif // ZURI_PROJECT_UPDATE_REQUIREMENTS_H