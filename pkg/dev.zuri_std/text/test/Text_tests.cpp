#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_config/workspace_config.h>
#include <tempo_test/tempo_test.h>

#include <zuri_std_text/config.h>

class StdText : public ::testing::Test {
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

TEST_F(StdText, TestEvaluateNewText)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/text" ...
        val text: Text = Text{"Hello, world!"}
        text
    )", options);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        DataCellRef(
            lyric_common::SymbolUrl(
                lyric_common::ModuleLocation::fromString(ZURI_STD_TEXT_LOCATION),
                lyric_common::SymbolPath({"Text"}))))));
}

TEST_F(StdText, TestEvaluateTextSize)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/text" ...
        val text: Text = Text{"Hello, world!"}
        text.Length()
    )", options);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(13))));
}

TEST_F(StdText, TestEvaluateTextAt)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/text" ...
        val text: Text = Text{"Hello, world!"}
        text.At(0)
    )", options);

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellChar(static_cast<UChar32>('H')))));
}