#ifndef ZURI_TOOLING_BUILD_TOOL_CONFIG_H
#define ZURI_TOOLING_BUILD_TOOL_CONFIG_H

#include <lyric_build/lyric_builder.h>
#include <lyric_build/task_settings.h>
#include <tempo_config/config_types.h>
#include <tempo_utils/status.h>

namespace zuri_tooling {

    class BuildToolConfig {
    public:
        explicit BuildToolConfig(const tempo_config::ConfigMap &buildMap);

        tempo_utils::Status configure();

        lyric_build::CacheMode getCacheMode() const;
        std::filesystem::path getBootstrapDirectory() const;
        absl::Duration getWaitTimeout() const;
        int getJobParallelism() const;
        lyric_build::TaskSettings getTaskSettings() const;

    private:
        tempo_config::ConfigMap m_buildMap;

        lyric_build::CacheMode m_cacheMode;
        std::filesystem::path m_bootstrapDirectory;
        absl::Duration m_waitTimeout;
        int m_jobParallelism;
        lyric_build::TaskSettings m_taskSettings;
    };
}

#endif // ZURI_TOOLING_BUILD_TOOL_CONFIG_H