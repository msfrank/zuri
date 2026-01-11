
#include <tempo_config/config_builder.h>
#include <tempo_config/config_utils.h>
#include <tempo_config/merge_map.h>
#include <zuri_tooling/core_config.h>

#include "zuri_tooling/tooling_result.h"

zuri_tooling::CoreConfig::CoreConfig(
    const Distribution &distribution,
    const Home &home,
    const tempo_config::ConfigMap &configMap)
    : m_distribution(distribution),
      m_home(home),
      m_configMap(configMap)
{
    TU_ASSERT (distribution.isValid());
}

static tempo_utils::Result<tempo_config::ConfigMap>
load_core_override_config()
{
    const auto *value = std::getenv(zuri_tooling::kEnvOverrideCoreConfigName);
    if (value == nullptr)
        return tempo_config::ConfigMap{};
    tempo_config::ConfigNode overrideNode;
    TU_ASSIGN_OR_RETURN (overrideNode, tempo_config::read_config_string(value));
    TU_LOG_V << "parsed env override config: " << overrideNode.toString();

    if (overrideNode.getNodeType() != tempo_config::ConfigNodeType::kMap)
        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kWrongType,
            "invalid type for environment variable {}; expected a map",
            zuri_tooling::kEnvOverrideCoreConfigName);

    return tempo_config::startMap()
        .put("zuri", tempo_config::startMap()
            .put("core", overrideNode).buildNode())
        .buildMap();
}

static tempo_utils::Result<tempo_config::ConfigMap>
read_core_config(
    const zuri_tooling::Distribution &distribution,
    const zuri_tooling::Home &home)
{
    tempo_config::ConfigMap distributionConfig;
    tempo_config::ConfigMap homeConfig;
    tempo_config::ConfigMap overrideConfig;

    // parse the distribution config
    auto distributionConfigDirectory = distribution.getConfigDirectory();
    TU_ASSIGN_OR_RETURN (distributionConfig, tempo_config::read_config_tree_directory(
        distributionConfigDirectory, ".config"));

    // parse the home config if present
    if (home.isValid()) {
        auto homeConfigDirectory = home.getConfigDirectory();
        TU_ASSIGN_OR_RETURN (homeConfig, tempo_config::read_config_tree_directory(
            homeConfigDirectory, ".config"));
    }

    // load override config if present
    TU_ASSIGN_OR_RETURN (overrideConfig, load_core_override_config());

    // return the core config
    return tempo_config::merge_map(distributionConfig,
        tempo_config::merge_map(homeConfig, overrideConfig));
}

tempo_utils::Status
zuri_tooling::CoreConfig::configure()
{
    auto zuriMap = m_configMap.mapAt("zuri").toMap();
    if (zuriMap.isNil())
        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kMissingValue,
            "missing required config 'zuri'");

    auto coreMap = zuriMap.mapAt("core").toMap();
    if (zuriMap.isNil())
        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kMissingValue,
            "missing required key 'zuri.core'");

    // parse zuri.core.defaults
    m_defaultsMap = coreMap.mapAt("defaults").toMap();
    if (m_defaultsMap.getNodeType() == tempo_config::ConfigNodeType::kMap) {

        // parse zuri.core.defaults.build
        auto buildDefaultsMap = m_defaultsMap.mapAt("build").toMap();
        m_defaultBuildConfig = std::make_shared<BuildToolConfig>(buildDefaultsMap);
        TU_RETURN_IF_NOT_OK (m_defaultBuildConfig->configure());
    }

    // parse zuri.core.logging
    auto loggingMap = coreMap.mapAt("logging").toMap();
    if (loggingMap.getNodeType() == tempo_config::ConfigNodeType::kMap) {
        m_loggingConfig = std::make_shared<LoggingConfig>(loggingMap);
        TU_RETURN_IF_NOT_OK (m_loggingConfig->configure());
    }

    return {};
}

tempo_utils::Result<std::shared_ptr<zuri_tooling::CoreConfig>>
zuri_tooling::CoreConfig::load(
    const Distribution &distribution,
    const Home &home)
{
    tempo_config::ConfigMap configMap;
    TU_ASSIGN_OR_RETURN (configMap, read_core_config(distribution, home));

    auto coreConfig = std::shared_ptr<CoreConfig>(new CoreConfig(
        distribution, home, configMap));
    TU_RETURN_IF_NOT_OK (coreConfig->configure());

    return coreConfig;
}

zuri_tooling::Distribution
zuri_tooling::CoreConfig::getDistribution() const
{
    return m_distribution;
}

zuri_tooling::Home
zuri_tooling::CoreConfig::getHome() const
{
    return m_home;
}

std::shared_ptr<zuri_tooling::LoggingConfig>
zuri_tooling::CoreConfig::getLoggingConfig() const
{
    return m_loggingConfig;
}

std::shared_ptr<zuri_tooling::BuildToolConfig>
zuri_tooling::CoreConfig::getDefaultBuildConfig() const
{
    return m_defaultBuildConfig;
}

tempo_config::ConfigMap
zuri_tooling::CoreConfig::getDefaultsMap() const
{
    return m_defaultsMap;
}
