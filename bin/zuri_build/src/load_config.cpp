/* SPDX-License-Identifier: AGPL-3.0-or-later */

#include <tempo_config/parse_config.h>
#include <tempo_utils/user_home.h>
#include <zuri_build/load_config.h>

static tempo_utils::Result<tempo_config::ConfigMap>
load_env_override_config()
{
    const auto *value = std::getenv(kEnvOverrideConfigName);
    if (value == nullptr)
        return tempo_config::ConfigMap();
    tempo_config::ConfigNode overrideNode;
    TU_ASSIGN_OR_RETURN (overrideNode, tempo_config::read_config_string(value));
    TU_LOG_V << "parsed env override config: " << overrideNode.toString();

    if (overrideNode.getNodeType() != tempo_config::ConfigNodeType::kMap)
        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kWrongType,
            "invalid type for environment variable {}; expected a map", kEnvOverrideConfigName);
    return overrideNode.toMap();
}

static tempo_utils::Result<tempo_config::ConfigMap>
load_env_override_vendor_config()
{
    const auto *value = std::getenv(kEnvOverrideVendorConfigName);
    if (value == nullptr)
        return tempo_config::ConfigMap();
    tempo_config::ConfigNode overrideNode;
    TU_ASSIGN_OR_RETURN (overrideNode, tempo_config::read_config_string(value));
    TU_LOG_V << "parsed env override vendor config: " << overrideNode.toString();

    if (overrideNode.getNodeType() != tempo_config::ConfigNodeType::kMap)
        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kWrongType,
            "invalid type for environment variable {}; expected a map", kEnvOverrideVendorConfigName);
    return overrideNode.toMap();
}

/**
 *
 * @param tool
 * @return
 */
tempo_utils::Result<std::shared_ptr<tempo_config::WorkspaceConfig>>
load_workspace_config(
    const std::filesystem::path &searchPathStart,
    const std::filesystem::path &distributionRoot)
{
    // load override config if present
    tempo_config::ConfigMap overrideConfig;
    TU_ASSIGN_OR_RETURN (overrideConfig, load_env_override_config());

    // load override vendor config if present
    tempo_config::ConfigMap overrideVendorConfig;
    TU_ASSIGN_OR_RETURN (overrideVendorConfig, load_env_override_vendor_config());

    tempo_config::WorkspaceConfigOptions workspaceOptions;
    workspaceOptions.toolLocator = {"zuri-build"};
    workspaceOptions.overrideWorkspaceConfigMap = overrideConfig;
    workspaceOptions.overrideVendorConfigMap = overrideVendorConfig;

    // set the dist paths
    workspaceOptions.distConfigDirectoryPath = distributionRoot / CONFIG_DIR_PREFIX;
    workspaceOptions.distVendorConfigDirectoryPath = distributionRoot / VENDOR_CONFIG_DIR_PREFIX;

    // determine the user's home directory
    auto userHome = tempo_utils::get_user_home_directory();

    // if home is found then set the user paths
    if (exists(userHome)) {
        auto userRoot = userHome / ".zuri";
        workspaceOptions.userConfigDirectoryPath = userRoot / tempo_config::kDefaultConfigDirectoryName;
        workspaceOptions.userVendorConfigDirectoryPath = userRoot / tempo_config::kDefaultVendorConfigDirectoryName;
    }

    return tempo_config::WorkspaceConfig::find(
        tempo_config::kDefaultWorkspaceConfigFileName, searchPathStart, workspaceOptions);
}
