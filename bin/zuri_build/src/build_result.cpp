
#include <absl/strings/substitute.h>

#include <zuri_build/build_result.h>

zuri_build::BuildStatus::BuildStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<BuildCondition>(statusCode, detail)
{
}

bool
zuri_build::BuildStatus::convert(BuildStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kZuriBuildStatusNs;
    if (srcNs != dstNs)
        return false;
    dstStatus = BuildStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}
