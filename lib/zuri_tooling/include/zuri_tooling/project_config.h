#ifndef ZURI_TOOLING_PROJECT_CONFIG_H
#define ZURI_TOOLING_PROJECT_CONFIG_H

#include <tempo_config/config_types.h>
#include <tempo_utils/result.h>

#include "core_config.h"
#include "environment_config.h"
#include "import_store.h"
#include "project.h"
#include "repository_store.h"
#include "resolver_config.h"
#include "target_store.h"

namespace zuri_tooling {

    /**
     *
     */
    constexpr const char * const kEnvOverrideProjectConfigName = "ZURI_OVERRIDE_PROJECT_CONFIG";

    /**
     *
     */
    class ProjectConfig {

    public:
        static tempo_utils::Result<std::shared_ptr<ProjectConfig>> load(
            const Project &project,
            std::shared_ptr<CoreConfig> coreConfig);

        Project getProject() const;
        std::shared_ptr<EnvironmentConfig> getEnvironmentConfig() const;
        std::shared_ptr<CoreConfig> getCoreConfig() const;

        std::shared_ptr<ImportStore> getImportStore() const;
        std::shared_ptr<TargetStore> getTargetStore() const;
        std::shared_ptr<BuildToolConfig> getBuildConfig() const;

    private:
        Project m_project;
        std::shared_ptr<EnvironmentConfig> m_environmentConfig;
        std::shared_ptr<CoreConfig> m_coreConfig;
        tempo_config::ConfigMap m_configMap;

        std::shared_ptr<ImportStore> m_importStore;
        std::shared_ptr<TargetStore> m_targetStore;
        std::shared_ptr<BuildToolConfig> m_buildConfig;

        ProjectConfig(
            const Project &project,
            std::shared_ptr<EnvironmentConfig> environmentConfig,
            std::shared_ptr<CoreConfig> coreConfig,
            const tempo_config::ConfigMap &configMap);

        tempo_utils::Status configure();
    };
}

#endif // ZURI_TOOLING_PROJECT_CONFIG_H