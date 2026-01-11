#ifndef ZURI_TOOLING_ENVIRONMENT_CONFIG_H
#define ZURI_TOOLING_ENVIRONMENT_CONFIG_H

#include <tempo_config/config_types.h>
#include <tempo_utils/result.h>

#include "core_config.h"
#include "environment.h"
#include "repository_store.h"
#include "resolver_config.h"

namespace zuri_tooling {

    /**
     *
     */
    constexpr const char * const kEnvOverrideEnvironmentConfigName = "ZURI_OVERRIDE_ENVIRONMENT_CONFIG";

    /**
     * Encapsulates the zuri.environment configuration loaded from the config directory in the specified
     * environment directory and core config. If ZURI_OVERRIDE_ENVIRONMENT_CONFIG is defined in the environment,
     * then the override config is merged on top.
     */
    class EnvironmentConfig {

    public:
        static tempo_utils::Result<std::shared_ptr<EnvironmentConfig>> load(
            const Environment &environment,
            std::shared_ptr<CoreConfig> coreConfig);

        Environment getEnvironment() const;
        std::shared_ptr<CoreConfig> getCoreConfig() const;

        std::shared_ptr<RepositoryStore> getRepositoryStore() const;

    private:
        Environment m_environment;
        std::shared_ptr<CoreConfig> m_coreConfig;
        tempo_config::ConfigMap m_configMap;

        std::shared_ptr<RepositoryStore> m_repositoryStore;
        std::shared_ptr<ResolverConfig> m_resolverConfig;

        EnvironmentConfig(
            const Environment &environment,
            std::shared_ptr<CoreConfig> coreConfig,
            const tempo_config::ConfigMap &configMap);

        tempo_utils::Status configure();
    };
}

#endif // ZURI_TOOLING_ENVIRONMENT_CONFIG_H