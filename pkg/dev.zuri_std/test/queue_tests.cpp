#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_config/workspace_config.h>
#include <tempo_test/tempo_test.h>
#include <zuri_test/zuri_tester.h>

class StdSystemQueue : public ::testing::Test {
protected:
    std::unique_ptr<zuri_test::ZuriTester> tester;

    void SetUp() override {
        zuri_test::TesterOptions options;
        options.localPackages.emplace_back(ZURI_STD_PACKAGE_PATH);
        tester = std::make_unique<zuri_test::ZuriTester>(options);
        TU_RAISE_IF_NOT_OK (tester->configure());
    }
};

TEST_F(StdSystemQueue, EvaluatePushAndAwaitPop)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/system" ...
        val queue: Queue[Int] = Queue[Int]{}
        queue.Push(42)
        Await(queue.Pop())
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(42))));
}

TEST_F(StdSystemQueue, EvaluatePushMultipleAndAwaitPop)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/system" ...
        val queue: Queue[Int] = Queue[Int]{}
        queue.Push(1)
        queue.Push(1)
        queue.Push(1)
        AwaitOrDefault(queue.Pop(), 0) + AwaitOrDefault(queue.Pop(), 0) + AwaitOrDefault(queue.Pop(), 0)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(3))));
}

TEST_F(StdSystemQueue, EvaluatePushMultipleAndAwaitPopInSeparateTasks)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/system" ...

        val queue: Queue[Int] = Queue[Int]{}

        val p1: Function0[Bool] = lambda(): Bool {
            queue.Push(1) and queue.Push(1) and queue.Push(1)
        }

        val p2: Function0[Int] = lambda(): Int {
            AwaitOrDefault(queue.Pop(), 0) + AwaitOrDefault(queue.Pop(), 0) + AwaitOrDefault(queue.Pop(), 0)
        }

        Spawn(p1)
        AwaitOrDefault(Spawn(p2), 0)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(3))));
}
