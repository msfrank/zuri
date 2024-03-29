#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_config/workspace_config.h>

class StdSystemElement : public ::testing::Test {
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

TEST_F(StdSystemElement, EvaluateElementWithMultipleChildren)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/system" ...
        val element: Element = Element{ns = `test`, id = 1, 1, 2, 3}
        element.children.GetOrElse(2, 0)
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(3)))));
}