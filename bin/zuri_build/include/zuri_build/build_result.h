#ifndef ZURI_BUILD_BUILD_RESULT_H
#define ZURI_BUILD_BUILD_RESULT_H

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/log_stream.h>
#include <tempo_utils/status.h>

namespace zuri_build {

    constexpr const char *kZuriBuildStatusNs = "dev.zuri.ns:zuri-build-status-1";

    enum class BuildCondition {
        kBuildInvariant,
    };

    class BuildStatus : public tempo_utils::TypedStatus<BuildCondition> {
    public:
        using TypedStatus::TypedStatus;
        static bool convert(BuildStatus &dstStatus, const tempo_utils::Status &srcStatus);

    private:
        BuildStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

    public:
        /**
         *
         * @param condition
         * @param message
         * @return
         */
        static BuildStatus forCondition(
            BuildCondition condition,
            std::string_view message)
        {
            return BuildStatus(condition, message);
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
        static BuildStatus forCondition(
            BuildCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return BuildStatus(condition, message);
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
        static BuildStatus forCondition(
            BuildCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return BuildStatus(condition, message, traceId, spanId);
        }
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<zuri_build::BuildCondition> {
        using ConditionType = zuri_build::BuildCondition;
        static bool convert(zuri_build::BuildStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return zuri_build::BuildStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<zuri_build::BuildCondition> {
        using StatusType = zuri_build::BuildStatus;
        static constexpr const char *condition_namespace() { return zuri_build::kZuriBuildStatusNs; }
        static constexpr StatusCode make_status_code(zuri_build::BuildCondition condition)
        {
            switch (condition) {
                case zuri_build::BuildCondition::kBuildInvariant:
                    return StatusCode::kInternal;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(zuri_build::BuildCondition condition)
        {
            switch (condition) {
                case zuri_build::BuildCondition::kBuildInvariant:
                    return "Build invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif // ZURI_BUILD_BUILD_RESULT_H