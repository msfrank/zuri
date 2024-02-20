#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_config/workspace_config.h>

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
        val op: EmitOperation = EmitOperation{value = 42}
        op.value
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(42)))));
}

TEST_F(StdSystemOperation, EvaluateEmitOperationWithStringValue)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/system" ...
        val op: EmitOperation = EmitOperation{value = "hello, world!"}
        op.value
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(IsRefType(lyric_common::SymbolPath({"String"}))))));
}

TEST_F(StdSystemOperation, EvaluateEmitOperationWithUrlValue)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/system" ...
        val op: EmitOperation = EmitOperation{value = `https://zuri.dev`}
        op.value
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(IsRefType(lyric_common::SymbolPath({"Url"}))))));
}

TEST_F(StdSystemOperation, EvaluateAppendOperationWithIntrinsicValue)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/system" ...
        val op: AppendOperation = AppendOperation{path = "/", value = 42}
        op.value
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(42)))));
}

TEST_F(StdSystemOperation, EvaluateInsertOperationWithIntrinsicValue)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/system" ...
        val op: InsertOperation = InsertOperation{path = "/", index = 0, value = 42}
        op.value
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(42)))));
}

TEST_F(StdSystemOperation, EvaluateUpdateOperationWithIntrinsicValue)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/system" ...
        val op: UpdateOperation = UpdateOperation{path = "/", ns = `foo`, id = 0, value = 42}
        op.value
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(42)))));
}

TEST_F(StdSystemOperation, EvaluateReplaceOperationWithIntrinsicValue)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/system" ...
        val op: ReplaceOperation = ReplaceOperation{path = "/", value = 42}
        op.value
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(42)))));
}
