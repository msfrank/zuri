#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_config/workspace_config.h>
#include <tempo_test/tempo_test.h>
#include <zuri_std_collections/config.h>

class StdOption : public ::testing::Test {
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

TEST_F(StdOption, TestEvaluateNewEmptyOption)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val opt: Option[Int] = Option[Int]{}
        opt
    )", options);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        DataCellRef(
            lyric_common::SymbolUrl(
                lyric_common::AssemblyLocation::fromString(ZURI_STD_COLLECTIONS_LOCATION),
                lyric_common::SymbolPath({"Option"}))))));
}

TEST_F(StdOption, TestEvaluateNewOption)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val opt: Option[Int] = Option[Int]{value = 42}
        opt
    )", options);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        DataCellRef(
            lyric_common::SymbolUrl(
                lyric_common::AssemblyLocation::fromString(ZURI_STD_COLLECTIONS_LOCATION),
                lyric_common::SymbolPath({"Option"}))))));
}

TEST_F(StdOption, TestEvaluateOptionIsEmpty)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val opt: Option[Int] = Option[Int]{}
        opt.IsEmpty()
    )", options);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(false))));
}

TEST_F(StdOption, TestEvaluateOptionGet)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val opt: Option[Int] = Option[Int]{value = 42}
        opt.Get()
    )", options);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(42))));
}

TEST_F(StdOption, TestEvaluateOptionGetOrElse)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val opt: Option[Int] = Option[Int]{}
        opt.GetOrElse(42)
    )", options);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(42))));
}
