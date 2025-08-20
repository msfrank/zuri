
#include <absl/strings/substitute.h>

#include <zuri_zpk/zpk_result.h>

zuri_zpk::ZpkStatus::ZpkStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<ZpkCondition>(statusCode, detail)
{
}

bool
zuri_zpk::ZpkStatus::convert(ZpkStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kZuriZpkStatusNs;
    if (srcNs != dstNs)
        return false;
    dstStatus = ZpkStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}
