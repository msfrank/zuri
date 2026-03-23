
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
    CacheTypeParser cacheTypeParser(CacheType::Memory);
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(m_cacheType, cacheTypeParser,
        m_buildMap, "cacheType"));

    // set the wait timeout if it was provided
    tempo_config::DurationParser waitTimeoutParser(absl::Duration{});
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(m_waitTimeout, waitTimeoutParser,
        m_buildMap, "waitTimeout"));

    // construct the task settings if it was provided
    auto settingsMap = m_buildMap.mapAt("settings").toMap();
    if (settingsMap.getNodeType() == tempo_config::ConfigNodeType::kMap) {
        m_taskSettings = lyric_build::TaskSettings(settingsMap);
    }

    return {};
}

zuri_tooling::CacheType
zuri_tooling::BuildToolConfig::getCacheType() const
{
    return m_cacheType;
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
