#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_config/workspace_config.h>
#include <tempo_test/tempo_test.h>

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

TEST_F(StdSystemAttr, EvaluateAttrWithDefaultValue) {
    auto result = lyric_test::LyricTester::runSingleModule(
        lyric_packaging::PackageSpecifier("test", "localhost", 0, 0, 0),
        R"(
        import from "//std/system" ...
        val attr: Attr = Attr{`test`, 1}
        attr.value
        )",
        options);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellUndef())));
}

TEST_F(StdSystemAttr, EvaluateAttrWithIntrinsicValue) {
    auto result = lyric_test::LyricTester::runSingleModule(
        lyric_packaging::PackageSpecifier("test", "localhost", 0, 0, 0),
        R"(
        import from "//std/system" ...
        val attr: Attr = Attr{`test`, 1, 42}
        attr.value
        )",
        options);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(42))));
}

TEST_F(StdSystemAttr, EvaluateAttrWithElementValue) {
    auto result = lyric_test::LyricTester::runSingleModule(
        lyric_packaging::PackageSpecifier("test", "localhost", 0, 0, 0),
        R"(
        import from "//std/system" ...
        val root: Element = Element{`io.fathomdata:ns:richtext-1`, 29, "Hello, world"}
        val attr: Attr = Attr{`test`, 1, root}
        attr.value
        )",
        options);

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellRef(lyric_common::SymbolPath({"Element"})))));
}
