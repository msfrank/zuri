#ifndef ZURI_RUN_RUN_RESULT_H
#define ZURI_RUN_RUN_RESULT_H

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/log_stream.h>
#include <tempo_utils/status.h>

namespace zuri_run {

    constexpr const char *kZuriRunStatusNs = "dev.zuri.ns:zuri-run-status-1";

    enum class RunCondition {
        kRunInvariant,
    };

    class RunStatus : public tempo_utils::TypedStatus<RunCondition> {
    public:
        using TypedStatus::TypedStatus;
        static bool convert(RunStatus &dstStatus, const tempo_utils::Status &srcStatus);

    private:
        RunStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

    public:
        /**
         *
         * @param condition
         * @param message
         * @return
         */
        static RunStatus forCondition(
            RunCondition condition,
            std::string_view message)
        {
            return RunStatus(condition, message);
        }
        /**
         *
         * @tparam Args
         * @param condition
         * @param messageFmt
         * @param messageArgs
         * @return
         */
        template <typename... Args>
        static RunStatus forCondition(
            RunCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return RunStatus(condition, message);
        }
        /**
         *
         * @tparam Args
         * @param condition
         * @param messageFmt
         * @param messageArgs
         * @return
         */
        template <typename... Args>
        static RunStatus forCondition(
            RunCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return RunStatus(condition, message, traceId, spanId);
        }
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<zuri_run::RunCondition> {
        using ConditionType = zuri_run::RunCondition;
        static bool convert(zuri_run::RunStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return zuri_run::RunStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<zuri_run::RunCondition> {
        using StatusType = zuri_run::RunStatus;
        static constexpr const char *condition_namespace() { return zuri_run::kZuriRunStatusNs; }
        static constexpr StatusCode make_status_code(zuri_run::RunCondition condition)
        {
            switch (condition) {
                case zuri_run::RunCondition::kRunInvariant:
                    return StatusCode::kInternal;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(zuri_run::RunCondition condition)
        {
            switch (condition) {
                case zuri_run::RunCondition::kRunInvariant:
                    return "Run invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif //ZURI_RUN_RUN_RESULT_H