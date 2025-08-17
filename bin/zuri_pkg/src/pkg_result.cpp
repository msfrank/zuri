
#include <absl/strings/substitute.h>

#include <zuri_pkg/pkg_result.h>

zuri_pkg::PkgStatus::PkgStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<PkgCondition>(statusCode, detail)
{
}

bool
zuri_pkg::PkgStatus::convert(PkgStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kZuriPkgStatusNs;
    if (srcNs != dstNs)
        return false;
    dstStatus = PkgStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}
