#ifndef ZURI_TOOLING_CORE_CONFIG_H
#define ZURI_TOOLING_CORE_CONFIG_H

#include <tempo_config/config_types.h>
#include <tempo_utils/result.h>

#include "build_tool_config.h"
#include "distribution.h"
#include "home.h"
#include "logging_config.h"

namespace zuri_tooling {

    /**
     *
     */
    constexpr const char * const kEnvOverrideCoreConfigName = "ZURI_OVERRIDE_CORE_CONFIG";

    /**
     * Encapsulates the zuri.core configuration loaded from distribution and home config directories.
     * If ZURI_OVERRIDE_CORE_CONFIG is defined in the environment, then the override config is merged
     * on top.
     */
    class CoreConfig {

    public:
        static tempo_utils::Result<std::shared_ptr<CoreConfig>> load(
            const Distribution &distribution,
            const Home &home = {});

        Distribution getDistribution() const;
        Home getHome() const;

        std::shared_ptr<LoggingConfig> getLoggingConfig() const;

        std::shared_ptr<BuildToolConfig> getDefaultBuildConfig() const;

    private:
        Distribution m_distribution;
        Home m_home;
        tempo_config::ConfigMap m_configMap;

        std::shared_ptr<LoggingConfig> m_loggingConfig;
        std::shared_ptr<BuildToolConfig> m_defaultBuildConfig;
        tempo_config::ConfigMap m_defaultsMap;

        CoreConfig(
            const Distribution &distribution,
            const Home &home,
            const tempo_config::ConfigMap &configMap);

        tempo_utils::Status configure();
        tempo_config::ConfigMap getDefaultsMap() const;

        friend class EnvironmentConfig;
        friend class ProjectConfig;
    };
}

#endif // ZURI_TOOLING_CORE_CONFIG_H