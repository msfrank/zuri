// #ifndef ZURI_PACKAGER_INTERNAL_TRACING_ERROR_LISTENER_H
// #define ZURI_PACKAGER_INTERNAL_TRACING_ERROR_LISTENER_H
//
// #include <memory>
//
// #include <antlr4-runtime.h>
//
// #include <tempo_tracing/trace_span.h>
//
// namespace zuri_packager::internal {
//
//     class RequirementListener;
//
//     class TracingErrorListener : public antlr4::BaseErrorListener {
//     public:
//         explicit TracingErrorListener(RequirementListener *listener);
//
//         void syntaxError(
//             antlr4::Recognizer *recognizer,
//             antlr4::Token *offendingSymbol,
//             size_t line,
//             size_t charPositionInLine,
//             const std::string &message,
//             std::exception_ptr e) override;
//
//     private:
//         RequirementListener *m_listener;
//     };
// }
//
// #endif // ZURI_PACKAGER_INTERNAL_TRACING_ERROR_LISTENER_H
