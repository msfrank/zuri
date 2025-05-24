
#include <absl/strings/substitute.h>

#include <zuri_packager/package_result.h>

zuri_packager::PackageStatus::PackageStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<PackageCondition>(statusCode, detail)
{
}

zuri_packager::PackageStatus
zuri_packager::PackageStatus::ok()
{
    return PackageStatus();
}

bool
zuri_packager::PackageStatus::convert(PackageStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kLyricPackagingStatusNs;
    if (srcNs != dstNs)
        return false;
    dstStatus = PackageStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}
