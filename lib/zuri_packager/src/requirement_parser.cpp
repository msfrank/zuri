//
// #include <tempo_tracing/trace_context.h>
// #include <tempo_tracing/enter_scope.h>
// #include <zuri_packager/internal/requirement_listener.h>
// #include <zuri_packager/internal/tracing_error_listener.h>
// #include <zuri_packager/packager_result.h>
// #include <zuri_packager/requirement_parser.h>
//
// #include <antlr4-runtime.h>
//
// #include "RequirementLexer.h"
// #include "RequirementParser.h"
//
// zuri_packager::RequirementParser::RequirementParser(const RequirementParserOptions &options)
//     : m_options(options)
// {
// }
//
// tempo_utils::Result<std::shared_ptr<zuri_packager::AbstractPackageRequirement>>
// zuri_packager::RequirementParser::parseRequirement(
//     std::string_view utf8,
//     std::shared_ptr<tempo_tracing::TraceRecorder> recorder) const
// {
//     if (utf8.empty())
//         return PackagerStatus::forCondition(PackagerCondition::kPackagerInvariant,
//             "empty requirement string");
//
//     antlr4::ANTLRInputStream input(utf8.data(), (size_t) utf8.size());
//     zpr1::RequirementLexer lexer(&input);
//     antlr4::CommonTokenStream tokens(&lexer);
//     zpr1::RequirementParser parser(&tokens);
//
//     // create the trace context
//     std::shared_ptr<tempo_tracing::TraceContext> context;
//     if (recorder != nullptr) {
//         TU_ASSIGN_OR_RETURN (context, tempo_tracing::TraceContext::makeUnownedContextAndSwitch(recorder));
//     } else {
//         TU_ASSIGN_OR_RETURN (context, tempo_tracing::TraceContext::makeContextAndSwitch());
//     }
//
//     // ensure context is released
//     tempo_tracing::ReleaseContext releaser(context);
//
//     // create the root span
//     tempo_tracing::EnterScope scope("zuri_packager::RequirementParser");
//
//     // create the listener
//     internal::RequirementListener listener(context);
//
//     // create the error listener
//     internal::TracingErrorListener tracingErrorListener(&listener);
//     lexer.removeErrorListeners();
//     lexer.addErrorListener(&tracingErrorListener);
//     parser.removeErrorListeners();
//     parser.addErrorListener(&tracingErrorListener);
//
//     //
//     if (m_options.enableExtraDiagnostics) {
//         antlr4::DiagnosticErrorListener diagnosticErrorListener(!m_options.reportAllAmbiguities);
//         parser.addErrorListener(&diagnosticErrorListener);
//     }
//
//     try {
//         antlr4::tree::ParseTree *tree = parser.requirement();
//         antlr4::tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);
//     } catch (tempo_utils::StatusException &ex) {
//         return ex.getStatus();
//     } catch (antlr4::ParseCancellationException &ex) {
//         return PackagerStatus::forCondition(PackagerCondition::kPackagerInvariant, ex.what());
//     }
//
//     return listener.toRequirement();
// }
//
// tempo_utils::Result<zuri_packager::RequirementsList>
// zuri_packager::RequirementParser::parseRequirementsList(
//     std::string_view utf8,
//     std::shared_ptr<tempo_tracing::TraceRecorder> recorder) const
// {
//     if (utf8.empty())
//         return PackagerStatus::forCondition(PackagerCondition::kPackagerInvariant,
//             "empty requirements string");
//
//     antlr4::ANTLRInputStream input(utf8.data(), (size_t) utf8.size());
//     zpr1::RequirementLexer lexer(&input);
//     antlr4::CommonTokenStream tokens(&lexer);
//     zpr1::RequirementParser parser(&tokens);
//
//     // create the trace context
//     std::shared_ptr<tempo_tracing::TraceContext> context;
//     if (recorder != nullptr) {
//         TU_ASSIGN_OR_RETURN (context, tempo_tracing::TraceContext::makeUnownedContextAndSwitch(recorder));
//     } else {
//         TU_ASSIGN_OR_RETURN (context, tempo_tracing::TraceContext::makeContextAndSwitch());
//     }
//
//     // ensure context is released
//     tempo_tracing::ReleaseContext releaser(context);
//
//     // create the root span
//     tempo_tracing::EnterScope scope("zuri_packager::RequirementParser");
//
//     // create the listener
//     internal::RequirementListener listener(context);
//
//     // create the error listener
//     internal::TracingErrorListener tracingErrorListener(&listener);
//     lexer.removeErrorListeners();
//     lexer.addErrorListener(&tracingErrorListener);
//     parser.removeErrorListeners();
//     parser.addErrorListener(&tracingErrorListener);
//
//     //
//     if (m_options.enableExtraDiagnostics) {
//         antlr4::DiagnosticErrorListener diagnosticErrorListener(!m_options.reportAllAmbiguities);
//         parser.addErrorListener(&diagnosticErrorListener);
//     }
//
//     try {
//         antlr4::tree::ParseTree *tree = parser.requirementsList();
//         antlr4::tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);
//     } catch (tempo_utils::StatusException &ex) {
//         return ex.getStatus();
//     } catch (antlr4::ParseCancellationException &ex) {
//         return PackagerStatus::forCondition(PackagerCondition::kPackagerInvariant, ex.what());
//     }
//
//     return listener.toRequirementsList();
// }