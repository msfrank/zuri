/* SPDX-License-Identifier: AGPL-3.0-or-later */

#include <tempo_config/parse_config.h>
#include <tempo_config/program_config.h>
#include <tempo_config/workspace_config.h>
#include <tempo_utils/user_home.h>
#include <zuri_tooling/tooling_result.h>
#include <zuri_tooling/zuri_config.h>

zuri_tooling::ZuriConfig::ZuriConfig(
    const std::filesystem::path &distributionRoot,
    const std::filesystem::path &userRoot,
    const std::filesystem::path &workspaceConfigFile,
    const tempo_config::ConfigMap &zuriMap,
    const tempo_config::ConfigMap &vendorMap)
    : m_distributionRoot(distributionRoot),
      m_userRoot(userRoot),
      m_workspaceConfigFile(workspaceConfigFile),
      m_zuriMap(zuriMap),
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
configure_program_config_options(
    const std::filesystem::path &distributionRoot,
    const std::filesystem::path &userRoot,
    tempo_config::ProgramConfigOptions &programConfigOptions)
{
    // load override config if present
    tempo_config::ConfigMap overrideConfig;
    TU_ASSIGN_OR_RETURN (overrideConfig, load_env_override_config());

    // load override vendor config if present
    tempo_config::ConfigMap overrideVendorConfig;
    TU_ASSIGN_OR_RETURN (overrideVendorConfig, load_env_override_vendor_config());

    programConfigOptions.toolLocator = {"zuri"};
    programConfigOptions.overrideProgramConfigMap = overrideConfig;
    programConfigOptions.overrideVendorConfigMap = overrideVendorConfig;

    // set the distribution paths
    programConfigOptions.distConfigDirectoryPath = distributionRoot / CONFIG_DIR_PREFIX;
    programConfigOptions.distVendorConfigDirectoryPath = distributionRoot / VENDOR_CONFIG_DIR_PREFIX;

    // set the user paths
    if (!userRoot.empty()) {
        programConfigOptions.userConfigDirectoryPath = userRoot / tempo_config::kDefaultConfigDirectoryName;
        programConfigOptions.userVendorConfigDirectoryPath = userRoot / tempo_config::kDefaultVendorConfigDirectoryName;
    }

    return {};
}

static tempo_utils::Status
configure_workspace_config_options(
    const std::filesystem::path &distributionRoot,
    const std::filesystem::path &userRoot,
    tempo_config::WorkspaceConfigOptions &workspaceConfigOptions)
{
    // load override config if present
    tempo_config::ConfigMap overrideConfig;
    TU_ASSIGN_OR_RETURN (overrideConfig, load_env_override_config());

    // load override vendor config if present
    tempo_config::ConfigMap overrideVendorConfig;
    TU_ASSIGN_OR_RETURN (overrideVendorConfig, load_env_override_vendor_config());

    workspaceConfigOptions.toolLocator = {"zuri"};
    workspaceConfigOptions.overrideWorkspaceConfigMap = overrideConfig;
    workspaceConfigOptions.overrideVendorConfigMap = overrideVendorConfig;

    // set the distribution paths
    workspaceConfigOptions.distConfigDirectoryPath = distributionRoot / CONFIG_DIR_PREFIX;
    workspaceConfigOptions.distVendorConfigDirectoryPath = distributionRoot / VENDOR_CONFIG_DIR_PREFIX;

    // set the user paths
    if (!userRoot.empty()) {
        workspaceConfigOptions.userConfigDirectoryPath = userRoot / tempo_config::kDefaultConfigDirectoryName;
        workspaceConfigOptions.userVendorConfigDirectoryPath = userRoot / tempo_config::kDefaultVendorConfigDirectoryName;
    }

    return {};
}

/**
 * Load zuri config from the system distribution.
 *
 * @param distributionRootOverride Override the auto-detected distribution root.
 * @return
 */
tempo_utils::Result<std::shared_ptr<zuri_tooling::ZuriConfig>>
zuri_tooling::ZuriConfig::forSystem(const std::filesystem::path &distributionRootOverride)
{
    std::filesystem::path distributionRoot = !distributionRootOverride.empty()?
        distributionRootOverride : std::filesystem::path(DISTRIBUTION_ROOT);
    if (!std::filesystem::exists(distributionRoot))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "distribution root '{}' does not exist", distributionRoot.string());

    tempo_config::ProgramConfigOptions programConfigOptions;
    TU_RETURN_IF_NOT_OK (configure_program_config_options(
        distributionRoot, {}, programConfigOptions));

    std::shared_ptr<tempo_config::ProgramConfig> programConfig;
    TU_ASSIGN_OR_RETURN (programConfig, tempo_config::ProgramConfig::load(programConfigOptions));

    auto applicationMap = programConfig->getToolConfig();
    auto vendorMap = programConfig->getVendorConfig();

    auto zuriConfig = std::shared_ptr<ZuriConfig>(new ZuriConfig(
        distributionRoot, {}, {}, applicationMap, vendorMap));
    TU_RETURN_IF_NOT_OK (zuriConfig->configure());

    return zuriConfig;
}

/**
 * Load zuri config for the user.
 *
 * @param userHomeOverride Override the auto-detected user home directory.
 * @param distributionRootOverride Override the auto-detected distribution root.
 * @return
 */
tempo_utils::Result<std::shared_ptr<zuri_tooling::ZuriConfig>>
zuri_tooling::ZuriConfig::forUser(
    const std::filesystem::path &userHomeOverride,
    const std::filesystem::path &distributionRootOverride)
{
    std::filesystem::path distributionRoot = !distributionRootOverride.empty()?
        distributionRootOverride : std::filesystem::path(DISTRIBUTION_ROOT);
    if (!std::filesystem::exists(distributionRoot))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "distribution root '{}' does not exist", distributionRoot.string());

    std::filesystem::path userHome = !userHomeOverride.empty()?
        userHomeOverride : tempo_utils::get_user_home_directory();
    auto userRoot = userHome / kDefaultUserDirectoryName;
    if (!std::filesystem::exists(userRoot)) {
        userRoot.clear();
    }

    tempo_config::ProgramConfigOptions programConfigOptions;
    TU_RETURN_IF_NOT_OK (configure_program_config_options(
        distributionRoot, userRoot, programConfigOptions));

    std::shared_ptr<tempo_config::ProgramConfig> programConfig;
    TU_ASSIGN_OR_RETURN (programConfig, tempo_config::ProgramConfig::load(programConfigOptions));

    auto applicationMap = programConfig->getToolConfig();
    auto vendorMap = programConfig->getVendorConfig();

    auto zuriConfig = std::shared_ptr<ZuriConfig>(new ZuriConfig(
        distributionRoot, userRoot, {}, applicationMap, vendorMap));
    TU_RETURN_IF_NOT_OK (zuriConfig->configure());

    return zuriConfig;
}

/**
 * Load zuri config from the specified workspace.config file.
 *
 * @param workspaceConfigFile The path to the workspace.config file.
 * @param userHomeOverride Override the auto-detected user home directory.
 * @param distributionRootOverride Override the auto-detected distribution root.
 * @return
 */
tempo_utils::Result<std::shared_ptr<zuri_tooling::ZuriConfig>>
zuri_tooling::ZuriConfig::forWorkspace(
    const std::filesystem::path &workspaceConfigFile,
    const std::filesystem::path &userHomeOverride,
    const std::filesystem::path &distributionRootOverride)
{
    std::filesystem::path distributionRoot = !distributionRootOverride.empty()?
        distributionRootOverride : std::filesystem::path(DISTRIBUTION_ROOT);
    if (!std::filesystem::exists(distributionRoot))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "distribution root '{}' does not exist", distributionRoot.string());

    std::filesystem::path userHome = !userHomeOverride.empty()?
        userHomeOverride : tempo_utils::get_user_home_directory();
    auto userRoot = userHome / kDefaultUserDirectoryName;
    if (!std::filesystem::exists(userRoot)) {
        userRoot.clear();
    }

    tempo_config::WorkspaceConfigOptions workspaceConfigOptions;
    TU_RETURN_IF_NOT_OK (configure_workspace_config_options(
        distributionRoot, userRoot, workspaceConfigOptions));

    std::shared_ptr<tempo_config::WorkspaceConfig> workspaceConfig;
    TU_ASSIGN_OR_RETURN (workspaceConfig, tempo_config::WorkspaceConfig::load(
        workspaceConfigFile, workspaceConfigOptions));

    auto applicationMap = workspaceConfig->getToolConfig();
    auto vendorMap = workspaceConfig->getVendorConfig();

    auto zuriConfig = std::shared_ptr<ZuriConfig>(new ZuriConfig(
        distributionRoot, userRoot, workspaceConfigFile, applicationMap, vendorMap));
    TU_RETURN_IF_NOT_OK (zuriConfig->configure());

    return zuriConfig;
}


tempo_utils::Status
zuri_tooling::ZuriConfig::configure()
{
    if (m_zuriMap.getNodeType() != tempo_config::ConfigNodeType::kMap)
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "invalid 'zuri' config node; expected a map");

    auto importsMap = m_zuriMap.mapAt("imports").toMap();
    if (importsMap.getNodeType() == tempo_config::ConfigNodeType::kMap) {
        m_importStore = std::make_shared<ImportStore>(importsMap);
        TU_RETURN_IF_NOT_OK (m_importStore->configure());
    }

    auto targetsMap = m_zuriMap.mapAt("targets").toMap();
    if (targetsMap.getNodeType() == tempo_config::ConfigNodeType::kMap) {
        m_targetStore = std::make_shared<TargetStore>(targetsMap);
        TU_RETURN_IF_NOT_OK (m_targetStore->configure());
    }

    auto dcacheMap = m_zuriMap.mapAt("dcache").toMap();

    tempo_config::ConfigMap ucacheMap;
    tempo_config::ConfigMap icacheMap;
    tempo_config::ConfigMap tcacheMap;
    if (!m_userRoot.empty()) {
        ucacheMap = m_zuriMap.mapAt("ucache").toMap();
    }
    if (!m_workspaceConfigFile.empty()) {
        icacheMap = m_zuriMap.mapAt("icache").toMap();
        tcacheMap = m_zuriMap.mapAt("tcache").toMap();
    }

    m_packageStore = std::make_shared<PackageStore>(dcacheMap, ucacheMap, icacheMap, tcacheMap);
    TU_RETURN_IF_NOT_OK (m_packageStore->configure());

    return {};
}

std::shared_ptr<zuri_tooling::ImportStore>
zuri_tooling::ZuriConfig::getImportStore() const
{
    return m_importStore;
}

std::shared_ptr<zuri_tooling::TargetStore>
zuri_tooling::ZuriConfig::getTargetStore() const
{
    return m_targetStore;
}
