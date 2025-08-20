#ifndef ZURI_ZPK_ZPK_RESULT_H
#define ZURI_ZPK_ZPK_RESULT_H

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/log_stream.h>
#include <tempo_utils/status.h>

namespace zuri_zpk {

    constexpr const char *kZuriZpkStatusNs = "dev.zuri.ns:zuri-zpk-status-1";

    enum class ZpkCondition {
        kZpkInvariant,
    };

    class ZpkStatus : public tempo_utils::TypedStatus<ZpkCondition> {
    public:
        using TypedStatus::TypedStatus;
        static bool convert(ZpkStatus &dstStatus, const tempo_utils::Status &srcStatus);

    private:
        ZpkStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

    public:
        /**
         *
         * @param condition
         * @param message
         * @return
         */
        static ZpkStatus forCondition(
            ZpkCondition condition,
            std::string_view message)
        {
            return ZpkStatus(condition, message);
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
        static ZpkStatus forCondition(
            ZpkCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return ZpkStatus(condition, message);
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
        static ZpkStatus forCondition(
            ZpkCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return ZpkStatus(condition, message, traceId, spanId);
        }
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<zuri_zpk::ZpkCondition> {
        using ConditionType = zuri_zpk::ZpkCondition;
        static bool convert(zuri_zpk::ZpkStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return zuri_zpk::ZpkStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<zuri_zpk::ZpkCondition> {
        using StatusType = zuri_zpk::ZpkStatus;
        static constexpr const char *condition_namespace() { return zuri_zpk::kZuriZpkStatusNs; }
        static constexpr StatusCode make_status_code(zuri_zpk::ZpkCondition condition)
        {
            switch (condition) {
                case zuri_zpk::ZpkCondition::kZpkInvariant:
                    return StatusCode::kInternal;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(zuri_zpk::ZpkCondition condition)
        {
            switch (condition) {
                case zuri_zpk::ZpkCondition::kZpkInvariant:
                    return "Zpk invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif //ZURI_ZPK_ZPK_RESULT_H