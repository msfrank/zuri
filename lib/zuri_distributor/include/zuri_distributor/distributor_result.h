#ifndef ZURI_DISTRIBUTOR_DISTRIBUTOR_RESULT_H
#define ZURI_DISTRIBUTOR_DISTRIBUTOR_RESULT_H

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/log_stream.h>
#include <tempo_utils/status.h>

namespace zuri_distributor {

    constexpr const char *kZuriDistributorStatusNs = "dev.zuri.ns:zuri-distributor-status-1";

    enum class DistributorCondition {
        kDistributorInvariant,
    };

    class DistributorStatus : public tempo_utils::TypedStatus<DistributorCondition> {
    public:
        using TypedStatus::TypedStatus;
        static DistributorStatus ok();
        static bool convert(DistributorStatus &dstStatus, const tempo_utils::Status &srcStatus);

    private:
        DistributorStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

    public:
        /**
         *
         * @param condition
         * @param message
         * @return
         */
        static DistributorStatus forCondition(
            DistributorCondition condition,
            std::string_view message)
        {
            return DistributorStatus(condition, message);
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
        static DistributorStatus forCondition(
            DistributorCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return DistributorStatus(condition, message);
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
        static DistributorStatus forCondition(
            DistributorCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return DistributorStatus(condition, message, traceId, spanId);
        }
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<zuri_distributor::DistributorCondition> {
        using ConditionType = zuri_distributor::DistributorCondition;
        static bool convert(zuri_distributor::DistributorStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return zuri_distributor::DistributorStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<zuri_distributor::DistributorCondition> {
        using StatusType = zuri_distributor::DistributorStatus;
        static constexpr const char *condition_namespace() { return zuri_distributor::kZuriDistributorStatusNs; }
        static constexpr StatusCode make_status_code(zuri_distributor::DistributorCondition condition)
        {
            switch (condition) {
                case zuri_distributor::DistributorCondition::kDistributorInvariant:
                    return StatusCode::kInternal;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(zuri_distributor::DistributorCondition condition)
        {
            switch (condition) {
                case zuri_distributor::DistributorCondition::kDistributorInvariant:
                    return "Distributor invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif // ZURI_DISTRIBUTOR_DISTRIBUTOR_RESULT_H