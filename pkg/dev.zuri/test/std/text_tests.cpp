#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_config/workspace_config.h>
#include <tempo_test/tempo_test.h>
#include <zuri_test/zuri_tester.h>

#include "test_utils.h"

class StdText : public ::testing::Test {
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

TEST_F(StdText, EvaluateNewText)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/text" ...
        val text: Text = Text{"Hello, world!"}
        text
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        DataCellRef(
            lyric_common::SymbolUrl(
                lyric_common::ModuleLocation::fromString("dev.zuri.pkg://std-0.0.1@zuri.dev/text"),
                lyric_common::SymbolPath({"Text"}))))));
}

TEST_F(StdText, EvaluateTextSize)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/text" ...
        val text: Text = Text{"Hello, world!"}
        text.Length()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(13))));
}

TEST_F(StdText, EvaluateTextAt)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/text" ...
        val text: Text = Text{"Hello, world!"}
        text.At(0)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellChar(static_cast<char32_t>('H')))));
}