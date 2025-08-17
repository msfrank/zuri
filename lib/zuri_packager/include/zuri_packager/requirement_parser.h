// #ifndef ZURI_PACKAGER_REQUIREMENT_PARSER_H
// #define ZURI_PACKAGER_REQUIREMENT_PARSER_H
//
// #include <tempo_tracing/trace_recorder.h>
// #include <tempo_utils/result.h>
// #include <tempo_utils/url.h>
//
// #include "package_requirement.h"
// #include "package_types.h"
//
// namespace zuri_packager {
//
//     struct RequirementParserOptions {
//         bool enableExtraDiagnostics = false;
//         bool reportAllAmbiguities = false;
//     };
//
//     class RequirementParser {
//     public:
//         explicit RequirementParser(const RequirementParserOptions &options);
//
//         tempo_utils::Result<std::shared_ptr<AbstractPackageRequirement>> parseRequirement(
//             std::string_view utf8,
//             std::shared_ptr<tempo_tracing::TraceRecorder> recorder) const;
//
//         tempo_utils::Result<RequirementsList> parseRequirementsList(
//             std::string_view utf8,
//             std::shared_ptr<tempo_tracing::TraceRecorder> recorder) const;
//
//     private:
//         RequirementParserOptions m_options;
//     };
// }
//
// #endif // ZURI_PACKAGER_REQUIREMENT_PARSER_H
