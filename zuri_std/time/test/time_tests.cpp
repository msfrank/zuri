#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_config/workspace_config.h>

#include <zuri_std_time/config.h>

class StdTime : public ::testing::Test {
protected:
    lyric_test::TesterOptions options;

    void SetUp() override {
        auto loadWorkspaceResult = tempo_config::WorkspaceConfig::load(
            TESTER_CONFIG_PATH, {});
        ASSERT_TRUE (loadWorkspaceResult.isResult());
        auto config = loadWorkspaceResult.getResult();
        options.buildConfig = config->getToolConfig();
        options.buildVendorConfig = config->getVendorConfig();
    }
};

TEST_F(StdTime, EvaluateNow)
{
    auto before = absl::Now();

    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/time" ...
        val instant: Instant = Now()
        instant.ToEpochMillis()
    )", options);

    auto after = absl::Now();

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(MatchesDataCellType(lyric_runtime::DataCellType::I64)))));

    auto epochMillis = result.getResult().getReturn().value.data.i64;
    ASSERT_LE (ToUnixMillis(before), epochMillis);
    ASSERT_GE (ToUnixMillis(after), epochMillis);
}

TEST_F(StdTime, EvaluateParseTimezone)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/time" ...
        val tz: Timezone = ParseTimezone("America/Los_Angeles")
        tz
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(IsRefType(
            lyric_common::SymbolUrl(
                lyric_common::AssemblyLocation::fromString(ZURI_STD_TIME_LOCATION),
                lyric_common::SymbolPath({"Timezone"})))))));
}
