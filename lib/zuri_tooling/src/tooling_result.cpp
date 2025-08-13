
#include <absl/strings/substitute.h>

#include <zuri_tooling/tooling_result.h>

zuri_tooling::ToolingStatus::ToolingStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<ToolingCondition>(statusCode, detail)
{
}

bool
zuri_tooling::ToolingStatus::convert(ToolingStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kZuriToolingStatusNs;
    if (srcNs != dstNs)
        return false;
    dstStatus = ToolingStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}
