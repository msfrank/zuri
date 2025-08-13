/* SPDX-License-Identifier: AGPL-3.0-or-later */

#include <tempo_config/parse_config.h>
#include <tempo_config/workspace_config.h>
#include <tempo_utils/user_home.h>
#include <zuri_tooling/zuri_workspace.h>

#include "zuri_tooling/tooling_result.h"

zuri_tooling::ZuriWorkspace::ZuriWorkspace(
    const tempo_config::ConfigMap &workspaceMap,
    const tempo_config::ConfigMap &vendorMap)
    : m_workspaceMap(workspaceMap),
      m_vendorMap(vendorMap)
{
}

static tempo_utils::Result<tempo_config::ConfigMap>
load_env_override_config()
{
    const auto *value = std::getenv(zuri_tooling::kEnvOverrideConfigName);
    if (value == nullptr)
        return tempo_config::ConfigMap();
    tempo_config::ConfigNode overrideNode;
    TU_ASSIGN_OR_RETURN (overrideNode, tempo_config::read_config_string(value));
    TU_LOG_V << "parsed env override config: " << overrideNode.toString();

    if (overrideNode.getNodeType() != tempo_config::ConfigNodeType::kMap)
        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kWrongType,
            "invalid type for environment variable {}; expected a map", zuri_tooling::kEnvOverrideConfigName);
    return overrideNode.toMap();
}

static tempo_utils::Result<tempo_config::ConfigMap>
load_env_override_vendor_config()
{
    const auto *value = std::getenv(zuri_tooling::kEnvOverrideVendorConfigName);
    if (value == nullptr)
        return tempo_config::ConfigMap();
    tempo_config::ConfigNode overrideNode;
    TU_ASSIGN_OR_RETURN (overrideNode, tempo_config::read_config_string(value));
    TU_LOG_V << "parsed env override vendor config: " << overrideNode.toString();

    if (overrideNode.getNodeType() != tempo_config::ConfigNodeType::kMap)
        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kWrongType,
            "invalid type for environment variable {}; expected a map", zuri_tooling::kEnvOverrideVendorConfigName);
    return overrideNode.toMap();
}

static tempo_utils::Status
configure_workspace_config_options(
    const std::filesystem::path &distributionRoot,
    tempo_config::WorkspaceConfigOptions &workspaceConfigOptions)
{
    // load override config if present
    tempo_config::ConfigMap overrideConfig;
    TU_ASSIGN_OR_RETURN (overrideConfig, load_env_override_config());

    // load override vendor config if present
    tempo_config::ConfigMap overrideVendorConfig;
    TU_ASSIGN_OR_RETURN (overrideVendorConfig, load_env_override_vendor_config());

    workspaceConfigOptions.toolLocator = {"zuri-build"};
    workspaceConfigOptions.overrideWorkspaceConfigMap = overrideConfig;
    workspaceConfigOptions.overrideVendorConfigMap = overrideVendorConfig;

    // set the dist paths
    workspaceConfigOptions.distConfigDirectoryPath = distributionRoot / CONFIG_DIR_PREFIX;
    workspaceConfigOptions.distVendorConfigDirectoryPath = distributionRoot / VENDOR_CONFIG_DIR_PREFIX;

    // determine the user's home directory
    auto userHome = tempo_utils::get_user_home_directory();

    // if home is found then set the user paths
    if (exists(userHome)) {
        auto userRoot = userHome / ".zuri";
        workspaceConfigOptions.userConfigDirectoryPath = userRoot / tempo_config::kDefaultConfigDirectoryName;
        workspaceConfigOptions.userVendorConfigDirectoryPath = userRoot / tempo_config::kDefaultVendorConfigDirectoryName;
    }

    return {};
}

/**
 * Load the workspace config file from the specified path, using the specified distribution root.
 *
 * @param workspaceConfigFile The path to the workspace.config file.
 * @param distributionRoot The distribution root.
 * @return
 */
tempo_utils::Result<std::shared_ptr<zuri_tooling::ZuriWorkspace>>
zuri_tooling::ZuriWorkspace::load(
    const std::filesystem::path &workspaceConfigFile,
    const std::filesystem::path &distributionRoot)
{
    tempo_config::WorkspaceConfigOptions workspaceConfigOptions;
    TU_RETURN_IF_NOT_OK (configure_workspace_config_options(distributionRoot, workspaceConfigOptions));

    std::shared_ptr<tempo_config::WorkspaceConfig> workspaceConfig;
    TU_ASSIGN_OR_RETURN (workspaceConfig, tempo_config::WorkspaceConfig::load(
        workspaceConfigFile, workspaceConfigOptions));

    auto workspaceMap = workspaceConfig->getWorkspaceConfig();
    auto vendorMap = workspaceConfig->getVendorConfig();

    auto zuriWorkspace = std::shared_ptr<ZuriWorkspace>(new ZuriWorkspace(workspaceMap, vendorMap));
    TU_RETURN_IF_NOT_OK (zuriWorkspace->configure());

    return zuriWorkspace;
}

/**
 * Find the workspace config file by starting from the searchPathStart and traversing up the filesystem
 * hierarchy until a workspace.config file is found.
 *
 * @param searchPathStart The path where to start searching from.
 * @param distributionRoot The distribution root.
 * @return
 */
tempo_utils::Result<std::shared_ptr<zuri_tooling::ZuriWorkspace>>
zuri_tooling::ZuriWorkspace::find(
    const std::filesystem::path &searchPathStart,
    const std::filesystem::path &distributionRoot)
{
    tempo_config::WorkspaceConfigOptions workspaceConfigOptions;
    TU_RETURN_IF_NOT_OK (configure_workspace_config_options(distributionRoot, workspaceConfigOptions));

    std::shared_ptr<tempo_config::WorkspaceConfig> workspaceConfig;
    TU_ASSIGN_OR_RETURN (workspaceConfig, tempo_config::WorkspaceConfig::find(
        tempo_config::kDefaultWorkspaceConfigFileName, searchPathStart, workspaceConfigOptions));

    auto workspaceMap = workspaceConfig->getWorkspaceConfig();
    auto vendorMap = workspaceConfig->getVendorConfig();

    auto zuriWorkspace = std::shared_ptr<ZuriWorkspace>(new ZuriWorkspace(workspaceMap, vendorMap));
    TU_RETURN_IF_NOT_OK (zuriWorkspace->configure());

    return zuriWorkspace;
}

tempo_utils::Status
zuri_tooling::ZuriWorkspace::configure()
{
    if (!m_workspaceMap.mapContains("zuri"))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing 'zuri' config node");
    auto zuriMap = m_workspaceMap.mapAt("zuri").toMap();
    if (zuriMap.getNodeType() != tempo_config::ConfigNodeType::kMap)
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "invalid 'zuri' config node; expected a map");

    auto importsMap = zuriMap.mapAt("imports").toMap();
    if (importsMap.getNodeType() == tempo_config::ConfigNodeType::kMap) {
        m_importStore = std::make_shared<ImportStore>(importsMap);
        TU_RETURN_IF_NOT_OK (m_importStore->configure());
    }

    auto targetsMap = zuriMap.mapAt("targets").toMap();
    if (targetsMap.getNodeType() == tempo_config::ConfigNodeType::kMap) {
        m_targetStore = std::make_shared<TargetStore>(targetsMap);
        TU_RETURN_IF_NOT_OK (m_targetStore->configure());
    }

    return {};
}

std::shared_ptr<zuri_tooling::ImportStore>
zuri_tooling::ZuriWorkspace::getImportStore() const
{
    return m_importStore;
}

std::shared_ptr<zuri_tooling::TargetStore>
zuri_tooling::ZuriWorkspace::getTargetStore() const
{
    return m_targetStore;
}
