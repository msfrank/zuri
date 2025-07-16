#ifndef ZURI_BUILD_PROGRAM_STATUS_H
#define ZURI_BUILD_PROGRAM_STATUS_H

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/log_stream.h>
#include <tempo_utils/status.h>

constexpr const char *kZuriBuildProgramStatusNs = "dev.zuri.ns:zuri-build-program-status-1";

enum class ProgramCondition {
    ProgramError,
    ProgramInvariant,
};

class ProgramStatus : public tempo_utils::TypedStatus<ProgramCondition> {
public:
    using TypedStatus::TypedStatus;

    static bool convert(ProgramStatus &dstStatus, const tempo_utils::Status &srcStatus)
    {
        auto srcNs = srcStatus.getErrorCategory();
        if (srcNs != kZuriBuildProgramStatusNs)
            return false;
        dstStatus = ProgramStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
        return true;
    }

private:
    ProgramStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail)
        : TypedStatus(statusCode, detail)
    {}

public:
    /**
     *
     * @param condition
     * @param message
     * @return
     */
    static ProgramStatus forCondition(
        ProgramCondition condition,
        std::string_view message)
    {
        return ProgramStatus(condition, message);
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
    static ProgramStatus forCondition(
        ProgramCondition condition,
        fmt::string_view messageFmt = {},
        Args... messageArgs)
    {
        auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
        return ProgramStatus(condition, message);
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
    static ProgramStatus forCondition(
        ProgramCondition condition,
        tempo_utils::TraceId traceId,
        tempo_utils::SpanId spanId,
        fmt::string_view messageFmt = {},
        Args... messageArgs)
    {
        auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
        return ProgramStatus(condition, message, traceId, spanId);
    }
};

namespace tempo_utils {

    template<>
    struct StatusTraits<ProgramCondition> {
        using ConditionType = ProgramCondition;
        static bool convert(ProgramStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return ProgramStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<ProgramCondition> {
        using StatusType = ProgramStatus;
        static constexpr const char *condition_namespace() { return kZuriBuildProgramStatusNs; }
        static constexpr StatusCode make_status_code(ProgramCondition condition)
        {
            switch (condition) {
                case ProgramCondition::ProgramError:
                    return StatusCode::kFailedPrecondition;
                case ProgramCondition::ProgramInvariant:
                    return StatusCode::kInternal;
                default:
                    return StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(ProgramCondition condition)
        {
            switch (condition) {
                case ProgramCondition::ProgramError:
                    return "Program error";
                case ProgramCondition::ProgramInvariant:
                    return "Program invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif // ZURI_BUILD_PROGRAM_STATUS_H
