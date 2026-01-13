
#include <absl/strings/substitute.h>

#include <zuri_project/project_result.h>

zuri_project::ProjectStatus::ProjectStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<ProjectCondition>(statusCode, detail)
{
}

bool
zuri_project::ProjectStatus::convert(ProjectStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kZuriProjectStatusNs;
    if (srcNs != dstNs)
        return false;
    dstStatus = ProjectStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}
