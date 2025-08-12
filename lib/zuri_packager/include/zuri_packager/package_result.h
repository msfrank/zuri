#ifndef ZURI_PACKAGER_PACKAGE_RESULT_H
#define ZURI_PACKAGER_PACKAGE_RESULT_H

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/log_stream.h>
#include <tempo_utils/status.h>

namespace zuri_packager {

    constexpr const char *kZuriPackagerStatusNs = "dev.zuri.ns:zuri-packager-status-1";

    enum class PackageCondition {
        kInvalidHeader,
        kInvalidManifest,
        kMissingEntry,
        kDuplicateEntry,
        kDuplicateAttr,
        kDuplicateNamespace,
        kPackageInvariant,
    };

    class PackageStatus : public tempo_utils::TypedStatus<PackageCondition> {
    public:
        using TypedStatus::TypedStatus;
        static bool convert(PackageStatus &dstStatus, const tempo_utils::Status &srcStatus);

    private:
        PackageStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

    public:
        /**
         *
         * @param condition
         * @param message
         * @return
         */
        static PackageStatus forCondition(
            PackageCondition condition,
            std::string_view message)
        {
            return PackageStatus(condition, message);
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
        static PackageStatus forCondition(
            PackageCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return PackageStatus(condition, message);
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
        static PackageStatus forCondition(
            PackageCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return PackageStatus(condition, message, traceId, spanId);
        }
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<zuri_packager::PackageCondition> {
        using ConditionType = zuri_packager::PackageCondition;
        static bool convert(zuri_packager::PackageStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return zuri_packager::PackageStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<zuri_packager::PackageCondition> {
        using StatusType = zuri_packager::PackageStatus;
        static constexpr const char *condition_namespace() { return zuri_packager::kZuriPackagerStatusNs; }
        static constexpr StatusCode make_status_code(zuri_packager::PackageCondition condition)
        {
            switch (condition) {
                case zuri_packager::PackageCondition::kInvalidHeader:
                    return StatusCode::kInvalidArgument;
                case zuri_packager::PackageCondition::kInvalidManifest:
                    return StatusCode::kInvalidArgument;
                case zuri_packager::PackageCondition::kMissingEntry:
                    return StatusCode::kNotFound;
                case zuri_packager::PackageCondition::kDuplicateEntry:
                    return StatusCode::kInvalidArgument;
                case zuri_packager::PackageCondition::kDuplicateAttr:
                    return StatusCode::kInvalidArgument;
                case zuri_packager::PackageCondition::kDuplicateNamespace:
                    return StatusCode::kInvalidArgument;
                case zuri_packager::PackageCondition::kPackageInvariant:
                    return StatusCode::kInternal;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(zuri_packager::PackageCondition condition)
        {
            switch (condition) {
                case zuri_packager::PackageCondition::kInvalidHeader:
                    return "Invalid header";
                case zuri_packager::PackageCondition::kInvalidManifest:
                    return "Invalid manifest";
                case zuri_packager::PackageCondition::kMissingEntry:
                    return "Missing entry";
                case zuri_packager::PackageCondition::kDuplicateEntry:
                    return "Duplicate entry";
                case zuri_packager::PackageCondition::kDuplicateAttr:
                    return "Duplicate attr";
                case zuri_packager::PackageCondition::kDuplicateNamespace:
                    return "Duplicate namespace";
                case zuri_packager::PackageCondition::kPackageInvariant:
                    return "Package invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif // ZURI_PACKAGER_PACKAGE_RESULT_H