/* SPDX-License-Identifier: AGPL-3.0-or-later */

#include <tempo_config/parse_config.h>
#include <tempo_config/program_config.h>
#include <tempo_config/workspace_config.h>
#include <tempo_utils/user_home.h>
#include <zuri_tooling/tooling_result.h>
#include <zuri_tooling/zuri_config.h>

zuri_tooling::ZuriConfig::ZuriConfig(
    const Distribution &distribution,
    const Home &home,
    const std::filesystem::path &workspaceConfigFile,
    const tempo_config::ConfigMap &zuriMap,
    const tempo_config::ConfigMap &vendorMap)
    : m_distribution(distribution),
      m_home(home),
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
    const zuri_tooling::Distribution &distribution,
    const zuri_tooling::Home &home,
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
    programConfigOptions.distConfigDirectoryPath = distribution.getConfigDirectory();
    programConfigOptions.distVendorConfigDirectoryPath = distribution.getVendorConfigDirectory();

    // set the user paths
    if (home.isValid()) {
        programConfigOptions.userConfigDirectoryPath = home.getConfigDirectory();
        programConfigOptions.userVendorConfigDirectoryPath = home.getVendorConfigDirectory();
    }

    return {};
}

static tempo_utils::Status
configure_workspace_config_options(
    const zuri_tooling::Distribution &distribution,
    const zuri_tooling::Home &home,
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
    workspaceConfigOptions.distConfigDirectoryPath = distribution.getConfigDirectory();
    workspaceConfigOptions.distVendorConfigDirectoryPath = distribution.getVendorConfigDirectory();

    // set the user paths
    if (home.isValid()) {
        workspaceConfigOptions.userConfigDirectoryPath = home.getConfigDirectory();
        workspaceConfigOptions.userVendorConfigDirectoryPath = home.getVendorConfigDirectory();
    }

    return {};
}

/**
 * Load zuri config from the system distribution.
 *
 * @param distribution The system distribution.
 * @return The Zuri configuration.
 */
tempo_utils::Result<std::shared_ptr<zuri_tooling::ZuriConfig>>
zuri_tooling::ZuriConfig::forSystem(const Distribution &distribution)
{
    tempo_config::ProgramConfigOptions programConfigOptions;
    TU_RETURN_IF_NOT_OK (configure_program_config_options(distribution, {}, programConfigOptions));

    std::shared_ptr<tempo_config::ProgramConfig> programConfig;
    TU_ASSIGN_OR_RETURN (programConfig, tempo_config::ProgramConfig::load(programConfigOptions));

    auto applicationMap = programConfig->getToolConfig();
    auto vendorMap = programConfig->getVendorConfig();

    auto zuriConfig = std::shared_ptr<ZuriConfig>(new ZuriConfig(
        distribution, {}, {}, applicationMap, vendorMap));
    TU_RETURN_IF_NOT_OK (zuriConfig->configure());

    return zuriConfig;
}

/**
 * Load zuri config for the user.
 *
 * @param home The user home.
 * @param distribution The system distribution.
 * @return The Zuri configuration.
 */
tempo_utils::Result<std::shared_ptr<zuri_tooling::ZuriConfig>>
zuri_tooling::ZuriConfig::forUser(
    const Home &home,
    const Distribution &distribution)
{
    tempo_config::ProgramConfigOptions programConfigOptions;
    TU_RETURN_IF_NOT_OK (configure_program_config_options(distribution, home, programConfigOptions));

    std::shared_ptr<tempo_config::ProgramConfig> programConfig;
    TU_ASSIGN_OR_RETURN (programConfig, tempo_config::ProgramConfig::load(programConfigOptions));

    auto applicationMap = programConfig->getToolConfig();
    auto vendorMap = programConfig->getVendorConfig();

    auto zuriConfig = std::shared_ptr<ZuriConfig>(new ZuriConfig(
        distribution, home, {}, applicationMap, vendorMap));
    TU_RETURN_IF_NOT_OK (zuriConfig->configure());

    return zuriConfig;
}

/**
 * Load zuri config from the specified workspace.config file.
 *
 * @param workspaceConfigFile The path to the workspace.config file.
 * @param home The user home.
 * @param distribution The system distribution.
 * @return The Zuri configuration.
 */
tempo_utils::Result<std::shared_ptr<zuri_tooling::ZuriConfig>>
zuri_tooling::ZuriConfig::forWorkspace(
    const std::filesystem::path &workspaceConfigFile,
    const Home &home,
    const Distribution &distribution)
{
    tempo_config::WorkspaceConfigOptions workspaceConfigOptions;
    TU_RETURN_IF_NOT_OK (configure_workspace_config_options(distribution, home, workspaceConfigOptions));

    std::shared_ptr<tempo_config::WorkspaceConfig> workspaceConfig;
    TU_ASSIGN_OR_RETURN (workspaceConfig, tempo_config::WorkspaceConfig::load(
        workspaceConfigFile, workspaceConfigOptions));

    auto applicationMap = workspaceConfig->getToolConfig();
    auto vendorMap = workspaceConfig->getVendorConfig();

    auto zuriConfig = std::shared_ptr<ZuriConfig>(new ZuriConfig(
        distribution, home, workspaceConfigFile, applicationMap, vendorMap));
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
    if (!m_home.isValid()) {
        ucacheMap = m_zuriMap.mapAt("ucache").toMap();
    }
    if (!m_workspaceConfigFile.empty()) {
        icacheMap = m_zuriMap.mapAt("icache").toMap();
        tcacheMap = m_zuriMap.mapAt("tcache").toMap();
    }

    m_packageStore = std::make_shared<PackageStore>(dcacheMap, ucacheMap, icacheMap, tcacheMap);
    TU_RETURN_IF_NOT_OK (m_packageStore->configure());

    auto buildMap = m_zuriMap.mapAt("build").toMap();
    if (buildMap.getNodeType() == tempo_config::ConfigNodeType::kMap) {
        m_buildToolConfig = std::make_shared<BuildToolConfig>(buildMap);
        TU_RETURN_IF_NOT_OK (m_buildToolConfig->configure());
    }

    return {};
}

zuri_tooling::Distribution
zuri_tooling::ZuriConfig::getDistribution() const
{
    return m_distribution;
}

zuri_tooling::Home
zuri_tooling::ZuriConfig::getHome() const
{
    return m_home;
}

std::filesystem::path
zuri_tooling::ZuriConfig::getWorkspaceRoot() const
{
    return m_workspaceConfigFile.parent_path();
}

std::filesystem::path
zuri_tooling::ZuriConfig::getWorkspaceConfigFile() const
{
    return m_workspaceConfigFile;
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

std::shared_ptr<zuri_tooling::PackageStore>
zuri_tooling::ZuriConfig::getPackageStore() const
{
    return m_packageStore;
}

std::shared_ptr<zuri_tooling::BuildToolConfig>
zuri_tooling::ZuriConfig::getBuildToolConfig() const
{
    return m_buildToolConfig;
}