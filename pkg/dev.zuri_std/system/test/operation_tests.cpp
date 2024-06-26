#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_config/workspace_config.h>
#include <tempo_test/tempo_test.h>

class StdSystemOperation : public ::testing::Test {
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

TEST_F(StdSystemOperation, EvaluateEmitOperationWithIntrinsicValue)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/system" ...
        val op: EmitOperation = EmitOperation{42}
        op.value
    )", options);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(42))));
}

TEST_F(StdSystemOperation, EvaluateEmitOperationWithStringValue)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/system" ...
        val op: EmitOperation = EmitOperation{"hello, world!"}
        op.value
    )", options);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellString("hello, world!"))));
}

TEST_F(StdSystemOperation, EvaluateEmitOperationWithUrlValue)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/system" ...
        val op: EmitOperation = EmitOperation{`https://zuri.dev`}
        op.value
    )", options);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellUrl("https://zuri.dev"))));
}

TEST_F(StdSystemOperation, EvaluateAppendOperationWithIntrinsicValue)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/system" ...
        val op: AppendOperation = AppendOperation{"/", 42}
        op.value
    )", options);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(42))));
}

TEST_F(StdSystemOperation, EvaluateInsertOperationWithIntrinsicValue)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/system" ...
        val op: InsertOperation = InsertOperation{"/", 0, 42}
        op.value
    )", options);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(42))));
}

TEST_F(StdSystemOperation, EvaluateUpdateOperationWithIntrinsicValue)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/system" ...
        val op: UpdateOperation = UpdateOperation{"/", `foo`, 0, 42}
        op.value
    )", options);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(42))));
}

TEST_F(StdSystemOperation, EvaluateReplaceOperationWithIntrinsicValue)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/system" ...
        val op: ReplaceOperation = ReplaceOperation{"/", 42}
        op.value
    )", options);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(42))));
}
