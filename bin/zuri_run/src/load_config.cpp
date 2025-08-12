/* SPDX-License-Identifier: AGPL-3.0-or-later */

#include <tempo_config/parse_config.h>
#include <tempo_utils/user_home.h>
#include <zuri_run/load_config.h>

static tempo_utils::Result<tempo_config::ConfigMap>
load_env_override_config()
{
    const auto *value = std::getenv(zuri_run::kEnvOverrideConfigName);
    if (value == nullptr)
        return tempo_config::ConfigMap();
    auto readConfigResult = tempo_config::read_config_string(value);
    if (readConfigResult.isStatus())
        return readConfigResult.getStatus();
    auto overrideNode = readConfigResult.getResult();
    TU_LOG_V << "parsed env override config: " << overrideNode.toString();

    if (overrideNode.getNodeType() != tempo_config::ConfigNodeType::kMap)
        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kWrongType,
            "invalid type for environment variable {}; expected a map", zuri_run::kEnvOverrideConfigName);
    return overrideNode.toMap();
}

static tempo_utils::Result<tempo_config::ConfigMap>
load_env_override_vendor_config()
{
    const auto *value = std::getenv(zuri_run::kEnvOverrideVendorConfigName);
    if (value == nullptr)
        return tempo_config::ConfigMap();
    auto readConfigResult = tempo_config::read_config_string(value);
    if (readConfigResult.isStatus())
        return readConfigResult.getStatus();
    auto overrideNode = readConfigResult.getResult();
    TU_LOG_V << "parsed env override vendor config: " << overrideNode.toString();

    if (overrideNode.getNodeType() != tempo_config::ConfigNodeType::kMap)
        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kWrongType,
            "invalid type for environment variable {}; expected a map", zuri_run::kEnvOverrideVendorConfigName);
    return overrideNode.toMap();
}

/**
 *
 * @param tool
 * @return
 */
tempo_utils::Result<std::shared_ptr<tempo_config::WorkspaceConfig>>
zuri_run::load_workspace_config(
    const std::filesystem::path &searchPathStart,
    const std::filesystem::path &distributionRoot)
{
    // load override config if present
    auto loadOverrideConfigResult = load_env_override_config();
    if (loadOverrideConfigResult.isStatus())
        return loadOverrideConfigResult.getStatus();

    // load override vendor config if present
    auto loadOverrideVendorConfigResult = load_env_override_vendor_config();
    if (loadOverrideVendorConfigResult.isStatus())
        return loadOverrideVendorConfigResult.getStatus();

    tempo_config::WorkspaceConfigOptions workspaceOptions;
    workspaceOptions.toolLocator = {"zuri", "run"};
    workspaceOptions.overrideWorkspaceConfigMap = loadOverrideConfigResult.getResult();;
    workspaceOptions.overrideVendorConfigMap = loadOverrideVendorConfigResult.getResult();;

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

tempo_utils::Result<std::shared_ptr<tempo_config::ProgramConfig>>
zuri_run::load_program_config(const std::filesystem::path &distributionRoot)
{
    // load override config if present
    auto loadOverrideConfigResult = load_env_override_config();
    if (loadOverrideConfigResult.isStatus())
        return loadOverrideConfigResult.getStatus();

    // load override vendor config if present
    auto loadOverrideVendorConfigResult = load_env_override_vendor_config();
    if (loadOverrideVendorConfigResult.isStatus())
        return loadOverrideVendorConfigResult.getStatus();

    tempo_config::ProgramConfigOptions programOptions;
    programOptions.toolLocator = {"zuri", "run"};
    programOptions.overrideProgramConfigMap = loadOverrideConfigResult.getResult();
    programOptions.overrideVendorConfigMap = loadOverrideVendorConfigResult.getResult();;

    // set the dist paths
    programOptions.distConfigDirectoryPath = distributionRoot / CONFIG_DIR_PREFIX;
    programOptions.distVendorConfigDirectoryPath = distributionRoot / VENDOR_CONFIG_DIR_PREFIX;

    // determine the user's home directory
    auto userHome = tempo_utils::get_user_home_directory();

    // if home is found then set the user paths
    if (exists(userHome)) {
        auto userRoot = userHome / ".zuri";
        programOptions.userConfigDirectoryPath = userRoot / tempo_config::kDefaultConfigDirectoryName;
        programOptions.userVendorConfigDirectoryPath = userRoot / tempo_config::kDefaultVendorConfigDirectoryName;
    }

    return tempo_config::ProgramConfig::load(programOptions);
}
