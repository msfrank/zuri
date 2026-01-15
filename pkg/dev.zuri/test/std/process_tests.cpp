#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <lyric_runtime/url_ref.h>

#include <lyric_test/matchers.h>
#include <tempo_config/workspace_config.h>
#include <tempo_test/tempo_test.h>
#include <tempo_utils/log_console.h>
#include <zuri_test/zuri_tester.h>

#include "test_utils.h"

class StdProcess : public ::testing::Test {
protected:
    std::unique_ptr<zuri_test::ZuriTester> tester;

    void SetUp() override {
        auto runtime = get_global_test_runtime();
        zuri_test::TesterOptions options;
        options.mainArguments = { "arg0", "arg1", "arg2" };
        options.localPackages.emplace_back(ZURI_STD_PACKAGE_PATH);
        tester = std::make_unique<zuri_test::ZuriTester>(runtime, options);
        TU_RAISE_IF_NOT_OK (tester->configure());
    }
};

TEST_F(StdProcess, EvaluateProgramId)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/process" ...
        Process.ProgramId
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        lyric_test::matchers::MatchesDataCellType(lyric_runtime::DataCellType::STRING))));
    TU_CONSOLE_OUT << result.getResult().getInterpreterExit().mainReturn;
}

TEST_F(StdProcess, EvaluateProgramMain)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/process" ...
        Process.ProgramMain
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        lyric_test::matchers::MatchesDataCellType(lyric_runtime::DataCellType::URL))));
    auto mainReturn = result.getResult().getInterpreterExit().mainReturn;
    TU_CONSOLE_OUT << mainReturn;

    auto programMain = mainReturn.data.url->getUrl();
    ASSERT_EQ ("dev.zuri.tester", programMain.getScheme());
}

TEST_F(StdProcess, EvaluateArguments)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/process" ...
        Process.Arguments.At(2)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        lyric_test::matchers::DataCellString("arg2"))));
}
