#ifndef ZURI_ENV_ENV_RESULT_H
#define ZURI_ENV_ENV_RESULT_H

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/log_stream.h>
#include <tempo_utils/status.h>

namespace zuri_env {

    constexpr const char *kZuriEnvStatusNs = "dev.zuri.ns:zuri-env-status-1";

    enum class EnvCondition {
        kEnvInvariant,
    };

    class EnvStatus : public tempo_utils::TypedStatus<EnvCondition> {
    public:
        using TypedStatus::TypedStatus;
        static bool convert(EnvStatus &dstStatus, const tempo_utils::Status &srcStatus);

    private:
        EnvStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

    public:
        /**
         *
         * @param condition
         * @param message
         * @return
         */
        static EnvStatus forCondition(
            EnvCondition condition,
            std::string_view message)
        {
            return EnvStatus(condition, message);
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
        static EnvStatus forCondition(
            EnvCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return EnvStatus(condition, message);
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
        static EnvStatus forCondition(
            EnvCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return EnvStatus(condition, message, traceId, spanId);
        }
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<zuri_env::EnvCondition> {
        using ConditionType = zuri_env::EnvCondition;
        static bool convert(zuri_env::EnvStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return zuri_env::EnvStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<zuri_env::EnvCondition> {
        using StatusType = zuri_env::EnvStatus;
        static constexpr const char *condition_namespace() { return zuri_env::kZuriEnvStatusNs; }
        static constexpr StatusCode make_status_code(zuri_env::EnvCondition condition)
        {
            switch (condition) {
                case zuri_env::EnvCondition::kEnvInvariant:
                    return StatusCode::kInternal;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(zuri_env::EnvCondition condition)
        {
            switch (condition) {
                case zuri_env::EnvCondition::kEnvInvariant:
                    return "Env invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif //ZURI_ENV_ENV_RESULT_H