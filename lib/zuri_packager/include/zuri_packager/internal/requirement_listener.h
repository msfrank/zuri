// #ifndef ZURI_PACKAGER_INTERNAL_REQUIREMENT_LISTENER_H
// #define ZURI_PACKAGER_INTERNAL_REQUIREMENT_LISTENER_H
//
// #include <tempo_tracing/tempo_spanset.h>
// #include <tempo_tracing/trace_context.h>
//
// #include "RequirementParserBaseListener.h"
//
// #include "../package_requirement.h"
// #include "../package_types.h"
//
// namespace zuri_packager::internal {
//
//     class RequirementListener : public zpr1::RequirementParserBaseListener {
//
//     public:
//         explicit RequirementListener(std::shared_ptr<tempo_tracing::TraceContext> context);
//
//         void logErrorOrThrow(
//             size_t lineNr,
//             size_t columnNr,
//             const std::string &message);
//
//         bool hasError() const;
//
//         void enterRequirement(zpr1::RequirementParser::RequirementContext *ctx) override;
//         void exitRequirement(zpr1::RequirementParser::RequirementContext *ctx) override;
//
//         void exitExactVersionRequirement(zpr1::RequirementParser::ExactVersionRequirementContext *ctx) override;
//         void exitTildeRangeRequirement(zpr1::RequirementParser::TildeRangeRequirementContext *ctx) override;
//         void exitCaretRangeRequirement(zpr1::RequirementParser::CaretRangeRequirementContext *ctx) override;
//         void exitHyphenRangeRequirement(zpr1::RequirementParser::HyphenRangeRequirementContext *ctx) override;
//
//         tempo_utils::Result<std::shared_ptr<AbstractPackageRequirement>> toRequirement() const;
//         tempo_utils::Result<RequirementsList> toRequirementsList() const;
//
//     private:
//         std::shared_ptr<tempo_tracing::TraceContext> m_context;
//
//         std::vector<std::shared_ptr<AbstractPackageRequirement>> m_requirements;
//         tempo_utils::Status m_status;
//     };
// }
//
// #endif // ZURI_PACKAGER_INTERNAL_REQUIREMENT_LISTENER_H
