
#include <absl/strings/substitute.h>

#include <zuri_packager/packager_result.h>

zuri_packager::PackagerStatus::PackagerStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<PackagerCondition>(statusCode, detail)
{
}

bool
zuri_packager::PackagerStatus::convert(PackagerStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kZuriPackagerStatusNs;
    if (srcNs != dstNs)
        return false;
    dstStatus = PackagerStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}
