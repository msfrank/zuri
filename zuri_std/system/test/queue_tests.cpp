#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_config/workspace_config.h>

class StdSystemQueue : public ::testing::Test {
protected:
    lyric_test::TesterOptions testerOptions;

    void SetUp() override {
        auto loadWorkspaceResult = tempo_config::WorkspaceConfig::load(
            TESTER_CONFIG_PATH, {});
        ASSERT_TRUE (loadWorkspaceResult.isResult());
        auto config = loadWorkspaceResult.getResult();

        testerOptions.buildConfig = config->getToolConfig();
        testerOptions.buildVendorConfig = config->getVendorConfig();
    }
};

TEST_F(StdSystemQueue, EvaluatePushAndAwaitPop)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/system" ...
        val queue: Queue[Int] = Queue[Int]{}
        queue.push(42)
        Await(queue.pop())
    )", testerOptions);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(42)))));
}

TEST_F(StdSystemQueue, EvaluatePushMultipleAndAwaitPop)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/system" ...
        val queue: Queue[Int] = Queue[Int]{}
        queue.push(1)
        queue.push(1)
        queue.push(1)
        AwaitOrDefault(queue.pop(), 0) + AwaitOrDefault(queue.pop(), 0) + AwaitOrDefault(queue.pop(), 0)
    )", testerOptions);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(3)))));
}
