/* SPDX-License-Identifier: AGPL-3.0-or-later */

#include <tempo_config/parse_config.h>
#include <tempo_utils/user_home.h>
#include <zuri_build/workspace_config.h>

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
tempo_utils::Result<std::shared_ptr<tempo_config::WorkspaceConfig>>
load_workspace_config(
    const std::filesystem::path &workspaceConfigFile,
    const std::filesystem::path &distributionRoot)
{
    tempo_config::WorkspaceConfigOptions workspaceConfigOptions;
    TU_RETURN_IF_NOT_OK (configure_workspace_config_options(distributionRoot, workspaceConfigOptions));
    return tempo_config::WorkspaceConfig::load(workspaceConfigFile, workspaceConfigOptions);
}

/**
 * Find the workspace config file by starting from the searchPathStart and traversing up the filesystem
 * hierarchy until a workspace.config file is found.
 *
 * @param searchPathStart The path where to start searching from.
 * @param distributionRoot The distribution root.
 * @return
 */
tempo_utils::Result<std::shared_ptr<tempo_config::WorkspaceConfig>>
find_workspace_config(
    const std::filesystem::path &searchPathStart,
    const std::filesystem::path &distributionRoot)
{
    tempo_config::WorkspaceConfigOptions workspaceConfigOptions;
    TU_RETURN_IF_NOT_OK (configure_workspace_config_options(distributionRoot, workspaceConfigOptions));

    return tempo_config::WorkspaceConfig::find(
        tempo_config::kDefaultWorkspaceConfigFileName, searchPathStart, workspaceConfigOptions);
}
