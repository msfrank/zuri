
#include <tempo_config/config_builder.h>
#include <tempo_config/config_utils.h>
#include <tempo_config/merge_map.h>
#include <zuri_tooling/environment_config.h>

zuri_tooling::EnvironmentConfig::EnvironmentConfig(
    const Environment &environment,
    std::shared_ptr<CoreConfig> coreConfig,
    const tempo_config::ConfigMap &configMap)
    : m_environment(environment),
      m_coreConfig(std::move(coreConfig)),
      m_configMap(configMap)
{
    TU_ASSERT (m_environment.isValid());
    TU_ASSERT (m_coreConfig != nullptr);
}

static tempo_utils::Result<tempo_config::ConfigMap>
load_environment_override_config()
{
    const auto *value = std::getenv(zuri_tooling::kEnvOverrideEnvironmentConfigName);
    if (value == nullptr)
        return tempo_config::ConfigMap{};
    tempo_config::ConfigNode overrideNode;
    TU_ASSIGN_OR_RETURN (overrideNode, tempo_config::read_config_string(value));
    TU_LOG_V << "parsed environment override config: " << overrideNode.toString();

    if (overrideNode.getNodeType() != tempo_config::ConfigNodeType::kMap)
        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kWrongType,
            "invalid type for environment variable {}; expected a map",
            zuri_tooling::kEnvOverrideEnvironmentConfigName);

    return tempo_config::startMap()
        .put("zuri", tempo_config::startMap()
            .put("environment", overrideNode).buildNode())
        .buildMap();
}

static tempo_utils::Result<tempo_config::ConfigMap>
read_environment_config(
    const std::filesystem::path &environmentConfigDirectory,
    const tempo_config::ConfigMap &defaultsMap)
{
    tempo_config::ConfigMap environmentConfig;
    tempo_config::ConfigMap overrideConfig;

    // build the default config
    auto defaultConfig = tempo_config::startMap()
        .put("zuri", tempo_config::startMap()
            .put("environment", defaultsMap).buildNode())
        .buildMap();

    // parse the environment config directory
    TU_ASSIGN_OR_RETURN (environmentConfig, tempo_config::read_config_tree_directory(
        environmentConfigDirectory, ".config"));

    // load override config if present
    TU_ASSIGN_OR_RETURN (overrideConfig, load_environment_override_config());

    // return the environment config
    return tempo_config::merge_map(defaultConfig,
        tempo_config::merge_map(environmentConfig, overrideConfig));
}

tempo_utils::Status
zuri_tooling::EnvironmentConfig::configure()
{
    auto zuriMap = m_configMap.mapAt("zuri").toMap();
    if (zuriMap.isNil())
        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kMissingValue,
            "missing required config 'zuri'");

    auto environmentMap = zuriMap.mapAt("environment").toMap();
    if (zuriMap.isNil())
        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kMissingValue,
            "missing required key 'zuri.environment'");

    // parse zuri.environment.repositories
    auto repositoriesMap = environmentMap.mapAt("repositories").toMap();
    if (repositoriesMap.getNodeType() == tempo_config::ConfigNodeType::kMap) {
        m_repositoryStore = std::make_shared<RepositoryStore>(repositoriesMap);
        TU_RETURN_IF_NOT_OK (m_repositoryStore->configure());
    }

    // parse zuri.environment.resolver
    auto resolverMap = environmentMap.mapAt("resolver").toMap();
    if (resolverMap.getNodeType() == tempo_config::ConfigNodeType::kMap) {
        m_resolverConfig = std::make_shared<ResolverConfig>(resolverMap);
        TU_RETURN_IF_NOT_OK (m_resolverConfig->configure());
    }

    return {};
}

tempo_utils::Result<std::shared_ptr<zuri_tooling::EnvironmentConfig>>
zuri_tooling::EnvironmentConfig::load(
    const Environment &environment,
    std::shared_ptr<CoreConfig> coreConfig)
{
    auto environmentDirectory = environment.getEnvironmentDirectory();

    tempo_config::ConfigMap configMap;
    TU_ASSIGN_OR_RETURN (configMap, read_environment_config(environmentDirectory, coreConfig->getDefaultsMap()));

    auto environmentConfig = std::shared_ptr<EnvironmentConfig>(new EnvironmentConfig(
        environment, coreConfig, configMap));
    TU_RETURN_IF_NOT_OK (environmentConfig->configure());

    return environmentConfig;
}

zuri_tooling::Environment
zuri_tooling::EnvironmentConfig::getEnvironment() const
{
    return m_environment;
}

std::shared_ptr<zuri_tooling::CoreConfig>
zuri_tooling::EnvironmentConfig::getCoreConfig() const
{
    return m_coreConfig;
}
