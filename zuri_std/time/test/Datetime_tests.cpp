#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_config/workspace_config.h>

#include <zuri_std_time/config.h>

class StdTimeDatetime : public ::testing::Test {
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

TEST_F(StdTimeDatetime, TestEvaluateNewStdTimeDatetime)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/time" ...

        val tz: Timezone = ParseTimezone("America/Los_Angeles")
        val dt: Datetime = Datetime{Now(), tz}
        dt
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(IsRefType(
            lyric_common::SymbolUrl(
                lyric_common::AssemblyLocation::fromString(ZURI_STD_TIME_LOCATION),
                lyric_common::SymbolPath({"Datetime"})))))));
}
