#ifndef ZURI_PROJECT_PROJECT_RESULT_H
#define ZURI_PROJECT_PROJECT_RESULT_H

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/log_stream.h>
#include <tempo_utils/status.h>

namespace zuri_project {

    constexpr const char *kZuriProjectStatusNs = "dev.zuri.ns:zuri-project-status-1";

    enum class ProjectCondition {
        kProjectInvariant,
    };

    class ProjectStatus : public tempo_utils::TypedStatus<ProjectCondition> {
    public:
        using TypedStatus::TypedStatus;
        static bool convert(ProjectStatus &dstStatus, const tempo_utils::Status &srcStatus);

    private:
        ProjectStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

    public:
        /**
         *
         * @param condition
         * @param message
         * @return
         */
        static ProjectStatus forCondition(
            ProjectCondition condition,
            std::string_view message)
        {
            return ProjectStatus(condition, message);
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
        static ProjectStatus forCondition(
            ProjectCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return ProjectStatus(condition, message);
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
        static ProjectStatus forCondition(
            ProjectCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return ProjectStatus(condition, message, traceId, spanId);
        }
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<zuri_project::ProjectCondition> {
        using ConditionType = zuri_project::ProjectCondition;
        static bool convert(zuri_project::ProjectStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return zuri_project::ProjectStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<zuri_project::ProjectCondition> {
        using StatusType = zuri_project::ProjectStatus;
        static constexpr const char *condition_namespace() { return zuri_project::kZuriProjectStatusNs; }
        static constexpr StatusCode make_status_code(zuri_project::ProjectCondition condition)
        {
            switch (condition) {
                case zuri_project::ProjectCondition::kProjectInvariant:
                    return StatusCode::kInternal;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(zuri_project::ProjectCondition condition)
        {
            switch (condition) {
                case zuri_project::ProjectCondition::kProjectInvariant:
                    return "Project invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif //ZURI_PROJECT_PROJECT_RESULT_H