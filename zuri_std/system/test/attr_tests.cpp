#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_config/workspace_config.h>

class StdSystemAttr : public ::testing::Test {
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

TEST_F(StdSystemAttr, EvaluateAttrWithIntrinsicValue) {
    auto result = lyric_test::LyricTester::runSingleModule(
        lyric_packaging::PackageSpecifier("test", "localhost", 0, 0, 0),
        R"(
        import from "//std/system" ...
        val attr: Attr = Attr{ns = `test`, id = 1, 42}
        attr.value
        )",
        options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(42)))));
}