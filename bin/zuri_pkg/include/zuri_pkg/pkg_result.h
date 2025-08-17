#ifndef ZURI_PKG_PKG_RESULT_H
#define ZURI_PKG_PKG_RESULT_H

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/log_stream.h>
#include <tempo_utils/status.h>

namespace zuri_pkg {

    constexpr const char *kZuriPkgStatusNs = "dev.zuri.ns:zuri-pkg-status-1";

    enum class PkgCondition {
        kPkgInvariant,
    };

    class PkgStatus : public tempo_utils::TypedStatus<PkgCondition> {
    public:
        using TypedStatus::TypedStatus;
        static bool convert(PkgStatus &dstStatus, const tempo_utils::Status &srcStatus);

    private:
        PkgStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

    public:
        /**
         *
         * @param condition
         * @param message
         * @return
         */
        static PkgStatus forCondition(
            PkgCondition condition,
            std::string_view message)
        {
            return PkgStatus(condition, message);
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
        static PkgStatus forCondition(
            PkgCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return PkgStatus(condition, message);
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
        static PkgStatus forCondition(
            PkgCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return PkgStatus(condition, message, traceId, spanId);
        }
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<zuri_pkg::PkgCondition> {
        using ConditionType = zuri_pkg::PkgCondition;
        static bool convert(zuri_pkg::PkgStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return zuri_pkg::PkgStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<zuri_pkg::PkgCondition> {
        using StatusType = zuri_pkg::PkgStatus;
        static constexpr const char *condition_namespace() { return zuri_pkg::kZuriPkgStatusNs; }
        static constexpr StatusCode make_status_code(zuri_pkg::PkgCondition condition)
        {
            switch (condition) {
                case zuri_pkg::PkgCondition::kPkgInvariant:
                    return StatusCode::kInternal;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(zuri_pkg::PkgCondition condition)
        {
            switch (condition) {
                case zuri_pkg::PkgCondition::kPkgInvariant:
                    return "Pkg invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif //ZURI_PKG_PKG_RESULT_H