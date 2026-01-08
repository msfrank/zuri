
#include <absl/strings/substitute.h>

#include <zuri_env/env_result.h>

zuri_env::EnvStatus::EnvStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<EnvCondition>(statusCode, detail)
{
}

bool
zuri_env::EnvStatus::convert(EnvStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kZuriEnvStatusNs;
    if (srcNs != dstNs)
        return false;
    dstStatus = EnvStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}
