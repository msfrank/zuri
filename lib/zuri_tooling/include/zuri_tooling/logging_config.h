#ifndef ZURI_TOOLING_LOGGING_CONFIG_H
#define ZURI_TOOLING_LOGGING_CONFIG_H

#include <tempo_config/config_types.h>
#include <tempo_utils/status.h>

namespace zuri_tooling {

    class LoggingConfig {
    public:
        explicit LoggingConfig(const tempo_config::ConfigMap &loggingMap);

        tempo_utils::Status configure();

        tempo_utils::SeverityFilter getSeverityFilter() const;
        bool getDisplayShortForm() const;
        bool getLogToStdout() const;
        bool getFlushEveryMessage() const;

    private:
        tempo_config::ConfigMap m_loggingMap;

        tempo_utils::SeverityFilter m_severityFilter;
        bool m_displayShortForm;
        bool m_logToStdout;
        bool m_flushEveryMessage;
    };
}

#endif // ZURI_TOOLING_LOGGING_CONFIG_H