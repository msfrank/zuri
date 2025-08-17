// #include <gtest/gtest.h>
// #include <gmock/gmock.h>
// #include <tempo_test/result_matchers.h>
//
// #include <zuri_packager/requirement_parser.h>
//
// TEST(RequirementParser, ParseExactVersionRequirement)
// {
//     zuri_packager::RequirementParserOptions options;
//     zuri_packager::RequirementParser parser(options);
//
//     auto recorder = tempo_tracing::TraceRecorder::create();
//     auto parseResult = parser.parseRequirement("1.2.3", recorder);
//     ASSERT_THAT (parseResult, tempo_test::IsResult());
//
//     auto req = parseResult.getResult();
//     ASSERT_EQ (zuri_packager::RequirementType::ExactVersion, req->getType());
//     ASSERT_EQ (tempo_config::valueNode("1.2.3"), req->toNode());
//
//     auto interval = req->getInterval();
//     ASSERT_EQ (interval.closedLowerBound, zuri_packager::PackageVersion(1, 2, 3));
//     ASSERT_EQ (interval.openUpperBound, zuri_packager::PackageVersion(1, 2, 4));
// }
//
// TEST(RequirementParser, ParseTildeRangeRequirement)
// {
//     zuri_packager::RequirementParserOptions options;
//     zuri_packager::RequirementParser parser(options);
//
//     auto recorder = tempo_tracing::TraceRecorder::create();
//     auto parseResult = parser.parseRequirement("~1.2.3", recorder);
//     ASSERT_THAT (parseResult, tempo_test::IsResult());
//
//     auto req = parseResult.getResult();
//     ASSERT_EQ (zuri_packager::RequirementType::TildeRange, req->getType());
//     ASSERT_EQ (tempo_config::valueNode("~1.2.3"), req->toNode());
//
//     auto interval = req->getInterval();
//     ASSERT_EQ (interval.closedLowerBound, zuri_packager::PackageVersion(1, 2, 3));
//     ASSERT_EQ (interval.openUpperBound, zuri_packager::PackageVersion(1, 3, 0));
// }
//
// TEST(RequirementParser, ParseCaretRangeRequirement)
// {
//     zuri_packager::RequirementParserOptions options;
//     zuri_packager::RequirementParser parser(options);
//
//     auto recorder = tempo_tracing::TraceRecorder::create();
//     auto parseResult = parser.parseRequirement("^1.2.3", recorder);
//     ASSERT_THAT (parseResult, tempo_test::IsResult());
//
//     auto req = parseResult.getResult();
//     ASSERT_EQ (zuri_packager::RequirementType::CaretRange, req->getType());
//     ASSERT_EQ (tempo_config::valueNode("^1.2.3"), req->toNode());
//
//     auto interval = req->getInterval();
//     ASSERT_EQ (interval.closedLowerBound, zuri_packager::PackageVersion(1, 2, 3));
//     ASSERT_EQ (interval.openUpperBound, zuri_packager::PackageVersion(2, 0, 0));
// }
//
// TEST(RequirementParser, ParseHyphenRangeRequirement)
// {
//     zuri_packager::RequirementParserOptions options;
//     options.enableExtraDiagnostics = true;
//     options.reportAllAmbiguities = true;
//     zuri_packager::RequirementParser parser(options);
//
//     auto recorder = tempo_tracing::TraceRecorder::create();
//     auto parseResult = parser.parseRequirement("1.2.3-1.2.6", recorder);
//     ASSERT_THAT (parseResult, tempo_test::IsResult());
//
//     auto req = parseResult.getResult();
//     ASSERT_EQ (zuri_packager::RequirementType::HyphenRange, req->getType());
//     ASSERT_EQ (tempo_config::valueNode("1.2.3-1.2.6"), req->toNode());
//
//     auto interval = req->getInterval();
//     ASSERT_EQ (interval.closedLowerBound, zuri_packager::PackageVersion(1, 2, 3));
//     ASSERT_EQ (interval.openUpperBound, zuri_packager::PackageVersion(1, 2, 7));
// }
