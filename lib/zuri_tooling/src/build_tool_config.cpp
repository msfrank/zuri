
#include <tempo_config/base_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_config/time_conversions.h>
#include <zuri_tooling/build_tool_config.h>
#include <zuri_tooling/tooling_conversions.h>

zuri_tooling::BuildToolConfig::BuildToolConfig(const tempo_config::ConfigMap &buildMap)
    : m_buildMap(buildMap)
{
}

tempo_utils::Status
zuri_tooling::BuildToolConfig::configure()
{
    // determine the job parallelism
    tempo_config::IntegerParser jobParallelismParser(0);
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(m_jobParallelism, jobParallelismParser,
        m_buildMap, "jobParallelism"));

    // determine the cache mode
    CacheModeParser cacheModeParser(lyric_build::CacheMode::Default);
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(m_cacheMode, cacheModeParser,
        m_buildMap, "cacheMode"));

    // set the wait timeout if it was provided
    tempo_config::DurationParser waitTimeoutParser(absl::Duration{});
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(m_waitTimeout, waitTimeoutParser,
        m_buildMap, "waitTimeout"));

    // set the bootstrap directory if it was provided
    tempo_config::PathParser bootstrapDirectoryParser(std::filesystem::path{});
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(m_bootstrapDirectory, bootstrapDirectoryParser,
        m_buildMap, "bootstrapDirectory"));

    // construct the task settings if it was provided
    auto settingsMap = m_buildMap.mapAt("settings").toMap();
    if (settingsMap.getNodeType() == tempo_config::ConfigNodeType::kMap) {
        m_taskSettings = lyric_build::TaskSettings(settingsMap);
    }

    return {};
}

lyric_build::CacheMode
zuri_tooling::BuildToolConfig::getCacheMode() const
{
    return m_cacheMode;
}

std::filesystem::path
zuri_tooling::BuildToolConfig::getBootstrapDirectory() const
{
    return m_bootstrapDirectory;
}

absl::Duration
zuri_tooling::BuildToolConfig::getWaitTimeout() const
{
    return m_waitTimeout;
}

int
zuri_tooling::BuildToolConfig::getJobParallelism() const
{
    return m_jobParallelism;
}

lyric_build::TaskSettings
zuri_tooling::BuildToolConfig::getTaskSettings() const
{
    return m_taskSettings;
}
