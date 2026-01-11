
#include <tempo_config/config_builder.h>
#include <tempo_config/config_utils.h>
#include <tempo_config/merge_map.h>
#include <zuri_tooling/environment_config.h>
#include <zuri_tooling/project_config.h>

zuri_tooling::ProjectConfig::ProjectConfig(
    const Project &project,
    std::shared_ptr<EnvironmentConfig> environmentConfig,
    std::shared_ptr<CoreConfig> coreConfig,
    const tempo_config::ConfigMap &configMap)
    : m_project(project),
      m_environmentConfig(std::move(environmentConfig)),
      m_coreConfig(std::move(coreConfig)),
      m_configMap(configMap)
{
    TU_ASSERT (m_project.isValid());
    TU_ASSERT (m_environmentConfig != nullptr);
    TU_ASSERT (m_coreConfig != nullptr);
}

static tempo_utils::Result<tempo_config::ConfigMap>
load_project_override_config()
{
    const auto *value = std::getenv(zuri_tooling::kEnvOverrideProjectConfigName);
    if (value == nullptr)
        return tempo_config::ConfigMap{};
    tempo_config::ConfigNode overrideNode;
    TU_ASSIGN_OR_RETURN (overrideNode, tempo_config::read_config_string(value));
    TU_LOG_V << "parsed project override config: " << overrideNode.toString();

    if (overrideNode.getNodeType() != tempo_config::ConfigNodeType::kMap)
        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kWrongType,
            "invalid type for environment variable {}; expected a map",
            zuri_tooling::kEnvOverrideProjectConfigName);

    return tempo_config::startMap()
        .put("zuri", tempo_config::startMap()
            .put("project", overrideNode).buildNode())
        .buildMap();
}

static tempo_utils::Result<tempo_config::ConfigMap>
read_project_config(
    const std::filesystem::path &projectConfigFile,
    const tempo_config::ConfigMap &defaultsMap)
{
    tempo_config::ConfigMap projectConfig;
    tempo_config::ConfigMap overrideConfig;

    // build the default config
    auto defaultConfig = tempo_config::startMap()
        .put("zuri", tempo_config::startMap()
            .put("project", defaultsMap).buildNode())
        .buildMap();

    // parse the project config file
    TU_ASSIGN_OR_RETURN (projectConfig, tempo_config::read_config_map_file(projectConfigFile));

    // load override config if present
    TU_ASSIGN_OR_RETURN (overrideConfig, load_project_override_config());

    // return the project config
    return tempo_config::merge_map(defaultConfig,
        tempo_config::merge_map(projectConfig, overrideConfig));
}

tempo_utils::Status
zuri_tooling::ProjectConfig::configure()
{
    auto zuriMap = m_configMap.mapAt("zuri").toMap();
    if (zuriMap.isNil())
        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kMissingValue,
            "missing required config 'zuri'");

    auto projectMap = zuriMap.mapAt("project").toMap();
    if (zuriMap.isNil())
        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kMissingValue,
            "missing required key 'zuri.project'");

    // parse zuri.project.imports
    auto importsMap = projectMap.mapAt("imports").toMap();
    if (importsMap.getNodeType() == tempo_config::ConfigNodeType::kMap) {
        m_importStore = std::make_shared<ImportStore>(importsMap);
        TU_RETURN_IF_NOT_OK (m_importStore->configure());
    }

    // parse zuri.project.targets
    auto targetsMap = projectMap.mapAt("targets").toMap();
    if (targetsMap.getNodeType() == tempo_config::ConfigNodeType::kMap) {
        m_targetStore = std::make_shared<TargetStore>(targetsMap);
        TU_RETURN_IF_NOT_OK (m_targetStore->configure());
    }

    // parse zuri.project.build
    auto buildMap = projectMap.mapAt("build").toMap();
    if (buildMap.getNodeType() == tempo_config::ConfigNodeType::kMap) {
        m_buildConfig = std::make_shared<BuildToolConfig>(buildMap);
        TU_RETURN_IF_NOT_OK (m_buildConfig->configure());
    }

    return {};
}

tempo_utils::Result<std::shared_ptr<zuri_tooling::ProjectConfig>>
zuri_tooling::ProjectConfig::load(
    const Project &project,
    std::shared_ptr<CoreConfig> coreConfig)
{
    auto environmentDirectory = project.getEnvironmentDirectory();
    Environment environment;
    TU_ASSIGN_OR_RETURN (environment, Environment::open(environmentDirectory));
    std::shared_ptr<EnvironmentConfig> environmentConfig;
    TU_ASSIGN_OR_RETURN(environmentConfig, EnvironmentConfig::load(environment, coreConfig));

    tempo_config::ConfigMap configMap;
    TU_ASSIGN_OR_RETURN (configMap, read_project_config(project.getProjectConfigFile(), coreConfig->getDefaultsMap()));

    auto projectConfig = std::shared_ptr<ProjectConfig>(new ProjectConfig(
        project, environmentConfig, coreConfig, configMap));
    TU_RETURN_IF_NOT_OK (projectConfig->configure());

    return projectConfig;
}

zuri_tooling::Project
zuri_tooling::ProjectConfig::getProject() const
{
    return m_project;
}

std::shared_ptr<zuri_tooling::EnvironmentConfig>
zuri_tooling::ProjectConfig::getEnvironmentConfig() const
{
    return m_environmentConfig;
}

std::shared_ptr<zuri_tooling::CoreConfig>
zuri_tooling::ProjectConfig::getCoreConfig() const
{
    return m_coreConfig;
}

std::shared_ptr<zuri_tooling::ImportStore>
zuri_tooling::ProjectConfig::getImportStore() const
{
    return m_importStore;
}

std::shared_ptr<zuri_tooling::TargetStore>
zuri_tooling::ProjectConfig::getTargetStore() const
{
    return m_targetStore;
}

std::shared_ptr<zuri_tooling::BuildToolConfig>
zuri_tooling::ProjectConfig::getBuildConfig() const
{
    return m_buildConfig;
}