
#include <RequirementParser.h>
#include <absl/strings/substitute.h>

#include <tempo_tracing/current_scope.h>
#include <tempo_tracing/enter_scope.h>
#include <tempo_tracing/exit_scope.h>
#include <tempo_tracing/span_log.h>
#include <tempo_tracing/tracing_schema.h>
#include <tempo_utils/log_message.h>
#include <zuri_packager/internal/requirement_listener.h>

#include "zuri_packager/package_result.h"
#include "zuri_packager/internal/version_spec.h"

zuri_packager::internal::RequirementListener::RequirementListener(
    std::shared_ptr<tempo_tracing::TraceContext> context)
    : m_context(std::move(context))
{
    TU_ASSERT (m_context != nullptr);
}

void
zuri_packager::internal::RequirementListener::logErrorOrThrow(
    size_t lineNr,
    size_t columnNr,
    const std::string &message)
{
    // // if this is the first error seen then set status
    // if (m_status.isOk()) {
    //     auto fullMessage = absl::Substitute(
    //         "parse error at $0:$1: $2", lineNr, columnNr, message);
    //     m_status = ConfigStatus::forCondition(
    //         ConfigCondition::kParseError, fullMessage);
    // }
}

bool
zuri_packager::internal::RequirementListener::hasError() const
{
    return m_status.notOk();
}

void
zuri_packager::internal::RequirementListener::enterRequirement(zpr1::RequirementParser::RequirementContext *ctx)
{
}

void
zuri_packager::internal::RequirementListener::exitRequirement(zpr1::RequirementParser::RequirementContext *ctx)
{
}


zuri_packager::internal::VersionSpec
parse_full_version_spec(zpr1::RequirementParser::FullVersionContext *ctx)
{
    auto numbers = ctx->WholeNumber();
    if (numbers.size() == 3) {
        tu_uint32 major, minor, patch;
        if (!absl::SimpleAtoi(numbers[0]->getText(), &major))
            return {};
        if (!absl::SimpleAtoi(numbers[1]->getText(), &minor))
            return {};
        if (!absl::SimpleAtoi(numbers[2]->getText(), &patch))
            return {};
        return zuri_packager::internal::VersionSpec(major, minor, patch);
    }
    return {};
}

zuri_packager::internal::VersionSpec
parse_api_version_spec(zpr1::RequirementParser::ApiVersionContext *ctx)
{
    auto numbers = ctx->WholeNumber();
    if (numbers.size() == 2) {
        tu_uint32 major, minor;
        if (!absl::SimpleAtoi(numbers[0]->getText(), &major))
            return {};
        if (!absl::SimpleAtoi(numbers[1]->getText(), &minor))
            return {};
        return zuri_packager::internal::VersionSpec(major, minor);
    }
    return {};
}

zuri_packager::internal::VersionSpec
parse_major_version_spec(zpr1::RequirementParser::MajorVersionContext *ctx)
{
    tu_uint32 major;
    if (!absl::SimpleAtoi(ctx->WholeNumber()->getText(), &major))
        return {};
    return zuri_packager::internal::VersionSpec(major);
}

zuri_packager::internal::VersionSpec
parse_version_spec(zpr1::RequirementParser::VersionContext *ctx)
{
    if (ctx->fullVersion())
        return parse_full_version_spec(ctx->fullVersion());
    if (ctx->apiVersion())
        return parse_api_version_spec(ctx->apiVersion());
    if (ctx->majorVersion())
        return parse_major_version_spec(ctx->majorVersion());
    return {};
}

void
zuri_packager::internal::RequirementListener::exitExactVersionRequirement(
    zpr1::RequirementParser::ExactVersionRequirementContext *ctx)
{
    auto spec = parse_version_spec(ctx->version());
    auto req = ExactVersionRequirement::create(spec.toVersion());
    m_requirements.push_back(std::move(req));
}

void
zuri_packager::internal::RequirementListener::exitTildeRangeRequirement(
    zpr1::RequirementParser::TildeRangeRequirementContext *ctx)
{
    auto spec = parse_version_spec(ctx->version());
    auto lower = spec.toVersion();

    std::shared_ptr<AbstractPackageRequirement> req;
    switch (spec.getType()) {
        case VersionSpecType::FullVersion:
            req = TildeRangeRequirement::create(lower.getMajorVersion(), lower.getMinorVersion(), lower.getPatchVersion());
            break;
        case VersionSpecType::ApiVersion:
            req = TildeRangeRequirement::create(lower.getMajorVersion(), lower.getMinorVersion());
            break;
        case VersionSpecType::MajorVersion:
            req = TildeRangeRequirement::create(lower.getMajorVersion());
            break;
        default:
            break;
    }

    m_requirements.push_back(std::move(req));
}

void
zuri_packager::internal::RequirementListener::exitCaretRangeRequirement(
    zpr1::RequirementParser::CaretRangeRequirementContext *ctx)
{
    auto spec = parse_version_spec(ctx->version());
    auto lower = spec.toVersion();

    std::shared_ptr<AbstractPackageRequirement> req;
    switch (spec.getType()) {
        case VersionSpecType::FullVersion:
            req = CaretRangeRequirement::create(lower.getMajorVersion(), lower.getMinorVersion(), lower.getPatchVersion());
            break;
        case VersionSpecType::ApiVersion:
            req = CaretRangeRequirement::create(lower.getMajorVersion(), lower.getMinorVersion());
            break;
        case VersionSpecType::MajorVersion:
            req = CaretRangeRequirement::create(lower.getMajorVersion());
            break;
        default:
            break;
    }

    m_requirements.push_back(std::move(req));
}

void
zuri_packager::internal::RequirementListener::exitHyphenRangeRequirement(
    zpr1::RequirementParser::HyphenRangeRequirementContext *ctx)
{
    auto lower = parse_version_spec(ctx->version(0)).toVersion();
    auto upper = parse_version_spec(ctx->version(1)).toVersion();
    auto req = HyphenRangeRequirement::create(lower, upper);
    m_requirements.push_back(std::move(req));
}

tempo_utils::Result<std::shared_ptr<zuri_packager::AbstractPackageRequirement>>
zuri_packager::internal::RequirementListener::toRequirement() const
{
    if (m_requirements.empty())
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "missing requirement");
    return m_requirements.back();
}

tempo_utils::Result<zuri_packager::RequirementsList>
zuri_packager::internal::RequirementListener::toRequirementsList() const
{
    TU_RETURN_IF_NOT_OK (m_status);
    if (m_requirements.empty())
        return RequirementsList{};
    return RequirementsList(m_requirements);
}