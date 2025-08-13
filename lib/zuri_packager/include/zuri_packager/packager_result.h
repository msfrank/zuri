#ifndef ZURI_PACKAGER_PACKAGER_RESULT_H
#define ZURI_PACKAGER_PACKAGER_RESULT_H

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/log_stream.h>
#include <tempo_utils/status.h>

namespace zuri_packager {

    constexpr const char *kZuriPackagerStatusNs = "dev.zuri.ns:zuri-packager-status-1";

    enum class PackagerCondition {
        kInvalidHeader,
        kInvalidManifest,
        kMissingEntry,
        kDuplicateEntry,
        kDuplicateAttr,
        kDuplicateNamespace,
        kPackagerInvariant,
    };

    class PackagerStatus : public tempo_utils::TypedStatus<PackagerCondition> {
    public:
        using TypedStatus::TypedStatus;
        static bool convert(PackagerStatus &dstStatus, const tempo_utils::Status &srcStatus);

    private:
        PackagerStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

    public:
        /**
         *
         * @param condition
         * @param message
         * @return
         */
        static PackagerStatus forCondition(
            PackagerCondition condition,
            std::string_view message)
        {
            return PackagerStatus(condition, message);
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
        static PackagerStatus forCondition(
            PackagerCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return PackagerStatus(condition, message);
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
        static PackagerStatus forCondition(
            PackagerCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return PackagerStatus(condition, message, traceId, spanId);
        }
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<zuri_packager::PackagerCondition> {
        using ConditionType = zuri_packager::PackagerCondition;
        static bool convert(zuri_packager::PackagerStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return zuri_packager::PackagerStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<zuri_packager::PackagerCondition> {
        using StatusType = zuri_packager::PackagerStatus;
        static constexpr const char *condition_namespace() { return zuri_packager::kZuriPackagerStatusNs; }
        static constexpr StatusCode make_status_code(zuri_packager::PackagerCondition condition)
        {
            switch (condition) {
                case zuri_packager::PackagerCondition::kInvalidHeader:
                    return StatusCode::kInvalidArgument;
                case zuri_packager::PackagerCondition::kInvalidManifest:
                    return StatusCode::kInvalidArgument;
                case zuri_packager::PackagerCondition::kMissingEntry:
                    return StatusCode::kNotFound;
                case zuri_packager::PackagerCondition::kDuplicateEntry:
                    return StatusCode::kInvalidArgument;
                case zuri_packager::PackagerCondition::kDuplicateAttr:
                    return StatusCode::kInvalidArgument;
                case zuri_packager::PackagerCondition::kDuplicateNamespace:
                    return StatusCode::kInvalidArgument;
                case zuri_packager::PackagerCondition::kPackagerInvariant:
                    return StatusCode::kInternal;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(zuri_packager::PackagerCondition condition)
        {
            switch (condition) {
                case zuri_packager::PackagerCondition::kInvalidHeader:
                    return "Invalid header";
                case zuri_packager::PackagerCondition::kInvalidManifest:
                    return "Invalid manifest";
                case zuri_packager::PackagerCondition::kMissingEntry:
                    return "Missing entry";
                case zuri_packager::PackagerCondition::kDuplicateEntry:
                    return "Duplicate entry";
                case zuri_packager::PackagerCondition::kDuplicateAttr:
                    return "Duplicate attr";
                case zuri_packager::PackagerCondition::kDuplicateNamespace:
                    return "Duplicate namespace";
                case zuri_packager::PackagerCondition::kPackagerInvariant:
                    return "Package invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif // ZURI_PACKAGER_PACKAGER_RESULT_H