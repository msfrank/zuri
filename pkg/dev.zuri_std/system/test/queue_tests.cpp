#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_config/workspace_config.h>
#include <tempo_test/tempo_test.h>

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
        queue.Push(42)
        Await(queue.Pop())
    )", testerOptions);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(42))));
}

TEST_F(StdSystemQueue, EvaluatePushMultipleAndAwaitPop)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/system" ...
        val queue: Queue[Int] = Queue[Int]{}
        queue.Push(1)
        queue.Push(1)
        queue.Push(1)
        AwaitOrDefault(queue.Pop(), 0) + AwaitOrDefault(queue.Pop(), 0) + AwaitOrDefault(queue.Pop(), 0)
    )", testerOptions);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(3))));
}

TEST_F(StdSystemQueue, EvaluatePushMultipleAndAwaitPopInSeparateTasks)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/system" ...
        val queue: Queue[Int] = Queue[Int]{}

        val p1: Function0[Bool] = lambda(): Bool {
            queue.Push(1) and queue.Push(1) and queue.Push(1)
        }

        val p2: Function0[Int] = lambda(): Int {
            AwaitOrDefault(queue.Pop(), 0) + AwaitOrDefault(queue.Pop(), 0) + AwaitOrDefault(queue.Pop(), 0)
        }

        Spawn(p1)
        AwaitOrDefault(Spawn(p2), 0)
    )", testerOptions);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(3))));
}
