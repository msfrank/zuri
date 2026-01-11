
#include <tempo_config/base_conversions.h>
#include <tempo_config/parse_config.h>
#include <zuri_tooling/logging_config.h>
#include <zuri_tooling/tooling_conversions.h>

zuri_tooling::LoggingConfig::LoggingConfig(const tempo_config::ConfigMap &loggingMap)
    : m_loggingMap(loggingMap)
{
}

tempo_utils::Status
zuri_tooling::LoggingConfig::configure()
{
    // determine the severity filter
    tempo_config::EnumTParser<tempo_utils::SeverityFilter> severityFilterParser({
        {"Default", tempo_utils::SeverityFilter::kDefault},
        {"Silent", tempo_utils::SeverityFilter::kSilent},
        {"ErrorsOnly", tempo_utils::SeverityFilter::kErrorsOnly},
        {"WarningsAndErrors", tempo_utils::SeverityFilter::kWarningsAndErrors},
        {"Verbose", tempo_utils::SeverityFilter::kVerbose},
        {"VeryVerbose", tempo_utils::SeverityFilter::kVeryVerbose},
    }, tempo_utils::SeverityFilter::kDefault);
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(m_severityFilter, severityFilterParser,
        m_loggingMap, "severityFilter"));

    // determine display short form
    tempo_config::BooleanParser displayShortFormParser(true);
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(m_displayShortForm, displayShortFormParser,
        m_loggingMap, "displayShortForm"));

    // determine log to stdout
    tempo_config::BooleanParser logToStdoutParser(false);
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(m_logToStdout, logToStdoutParser,
        m_loggingMap, "logToStdout"));

    // determine flush every message
    tempo_config::BooleanParser flushEveryMessageParser(false);
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(m_flushEveryMessage, flushEveryMessageParser,
        m_loggingMap, "flushEveryMessage"));

    return {};
}

tempo_utils::SeverityFilter
zuri_tooling::LoggingConfig::getSeverityFilter() const
{
    return m_severityFilter;
}

bool
zuri_tooling::LoggingConfig::getDisplayShortForm() const
{
    return m_displayShortForm;
}

bool
zuri_tooling::LoggingConfig::getLogToStdout() const
{
    return m_logToStdout;
}

bool
zuri_tooling::LoggingConfig::getFlushEveryMessage() const
{
    return m_flushEveryMessage;
}