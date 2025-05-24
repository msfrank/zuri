
#include <absl/strings/substitute.h>

#include <zuri_distributor/distributor_result.h>

zuri_distributor::DistributorStatus::DistributorStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<DistributorCondition>(statusCode, detail)
{
}

bool
zuri_distributor::DistributorStatus::convert(DistributorStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kZuriDistributorStatusNs;
    if (srcNs != dstNs)
        return false;
    dstStatus = DistributorStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}
