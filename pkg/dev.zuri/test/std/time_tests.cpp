#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_config/workspace_config.h>
#include <tempo_test/tempo_test.h>
#include <zuri_test/zuri_tester.h>

#include "test_utils.h"

class StdTime : public ::testing::Test {
protected:
    std::unique_ptr<zuri_test::ZuriTester> tester;

    void SetUp() override {
        auto runtime = get_global_test_runtime();
        zuri_test::TesterOptions options;
        options.localPackages.emplace_back(ZURI_STD_PACKAGE_PATH);
        tester = std::make_unique<zuri_test::ZuriTester>(runtime, options);
        TU_RAISE_IF_NOT_OK (tester->configure());
    }
};

TEST_F(StdTime, EvaluateNow)
{
    auto before = absl::Now();

    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/time" ...
        val instant: Instant = Now()
        instant.ToEpochMillis()
    )");

    auto after = absl::Now();

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        MatchesDataCellType(lyric_runtime::DataCellType::I64))));

    auto epochMillis = result.getResult().getInterpreterExit().mainReturn.data.i64;
    ASSERT_LE (ToUnixMillis(before), epochMillis);
    ASSERT_GE (ToUnixMillis(after), epochMillis);
}

TEST_F(StdTime, EvaluateParseTimezone)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/time" ...
        val tz: Timezone = ParseTimezone("America/Los_Angeles")
        tz
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        DataCellRef(
            lyric_common::SymbolUrl(
                lyric_common::ModuleLocation::fromString("dev.zuri.pkg://std-0.0.1@zuri.dev/time"),
                lyric_common::SymbolPath({"Timezone"}))))));
}
