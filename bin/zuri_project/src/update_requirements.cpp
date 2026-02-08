
#include <tempo_config/config_builder.h>
#include <zuri_project/update_requirements.h>

#include "zuri_project/project_result.h"


tempo_utils::Status
zuri_project::update_requirements(
    const UpdateRequirementsOperation &op,
    std::shared_ptr<zuri_tooling::ImportStore> importStore,
    tempo_config::ConfigFileEditor &projectConfigEditor)
{
    std::vector<zuri_packager::PackageSpecifier> importsToAdd;
    std::vector<zuri_packager::PackageSpecifier> importsToUpgrade;
    std::vector<zuri_packager::PackageId> importsToRemove;

    absl::flat_hash_map<zuri_packager::PackageId,zuri_packager::PackageVersion> requirementsSeen;

    // determine imports to add or upgrade
    for (const auto &requirement : op.addRequirements) {
        auto importEntry = importStore->getImport(requirement.getPackageId());
        auto requirementId = requirement.getPackageId();
        auto requirementVersion = requirement.getPackageVersion();

        if (requirementsSeen.contains(requirementId))
            return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
                "duplicate requirement for {}", requirementId.toString());

        // if import is not part of the project then add it, otherwise upgrade if compatible
        if (importEntry == nullptr) {
            requirementsSeen[requirementId] = requirementVersion;
            importsToAdd.push_back(requirement);
        } else {
            switch (importEntry->type) {
                case zuri_tooling::ImportEntryType::Version:
                    break;
                case zuri_tooling::ImportEntryType::Path:
                    return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
                        "cannot modify existing Path import for {}", requirementId.toString());
                default:
                    return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
                        "invalid import for {}", requirementId.toString());
            }

            if (requirement.getMajorVersion() != importEntry->version.getMajorVersion())
                return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
                    "incompatible major version for {}", requirementId.toString());

            auto cmp = requirement.getPackageVersion().compare(importEntry->version);
            if (cmp > 0) {
                requirementsSeen[requirementId] = requirementVersion;
                importsToUpgrade.push_back(requirement);
            }
        }
    }

    // determine imports to remove
    for (const auto &requirementId : op.removeRequirements) {
        auto importEntry = importStore->getImport(requirementId);
        if (importEntry == nullptr)
            continue;

        if (requirementsSeen.contains(requirementId))
            return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
                "duplicate requirement for {}", requirementId.toString());

        switch (importEntry->type) {
            case zuri_tooling::ImportEntryType::Version:
                break;
            case zuri_tooling::ImportEntryType::Path:
                return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
                    "cannot remove existing Path import for {}", requirementId.toString());
            default:
                return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
                    "invalid import for {}", requirementId.toString());
        }

        requirementsSeen[requirementId] = {};
        importsToRemove.push_back(requirementId);
    }

    // if no modifications are required then we are done
    if (importsToAdd.empty() && importsToUpgrade.empty() && importsToRemove.empty())
        return {};

    // sort the added imports lexographically
    std::sort(importsToAdd.begin(), importsToAdd.end());

    tempo_config::ConfigPath root;

    // if we are adding import entries and the imports node is not present in the project config then add it
    auto importsPath = root.traverse("imports");
    if (!importsToAdd.empty()) {
        if (!projectConfigEditor.hasNode(importsPath)) {
            TU_RETURN_IF_NOT_OK (projectConfigEditor.insertNode(importsPath, tempo_config::ConfigMap{}));
        }
    }

    // insert node for each added import
    for (const auto &specifier : importsToAdd) {
        auto importEntryPath = importsPath.traverse(specifier.getPackageId().toString());
        auto importVersion = tempo_config::valueNode(specifier.getPackageVersion().toString());
        TU_RETURN_IF_NOT_OK (projectConfigEditor.insertNode(importEntryPath, importVersion));
    }

    // replace node for each upgraded import
    for (const auto &specifier : importsToUpgrade) {
        auto importEntryPath = importsPath.traverse(specifier.getPackageId().toString());
        auto importVersion = tempo_config::valueNode(specifier.getPackageVersion().toString());
        TU_RETURN_IF_NOT_OK (projectConfigEditor.replaceNode(importEntryPath, importVersion));
    }

    // remove node for each removed import
    for (const auto &id : importsToRemove) {
        auto importEntryPath = importsPath.traverse(id.toString());
        TU_RETURN_IF_NOT_OK (projectConfigEditor.removeNode(importEntryPath));
    }

    return {};
}