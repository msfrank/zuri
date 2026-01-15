#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_config/workspace_config.h>
#include <tempo_test/tempo_test.h>
#include <zuri_test/zuri_tester.h>

#include "test_utils.h"

class StdSystemSystem : public ::testing::Test {
protected:
    std::unique_ptr<zuri_test::ZuriTester> tester;

    void SetUp() override {
        auto runtime = get_global_test_runtime();
        zuri_test::TesterOptions options;
        options.localPackages.emplace_back(ZURI_STD_PACKAGE_PATH);
        tester = std::make_unique<zuri_test::ZuriTester>(runtime, options);
        TU_RAISE_IF_NOT_OK (tester->configure());
    }
};

TEST_F(StdSystemSystem, EvaluateAwaitCompletedFuture)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/system" ...
        val fut: Future[Int] = Future[Int]{}
        fut.Complete(42)
        Await(fut)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(42))));
}

TEST_F(StdSystemSystem, EvaluateAwaitRejectedFuture)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/system" ...
        val fut: Future[Int] = Future[Int]{}
        val failure: Internal = Internal{message = "internal failure"}
        fut.Reject(failure)
        Await(fut)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        StatusRef(lyric_common::SymbolPath({"Internal"})))));
}

TEST_F(StdSystemSystem, EvaluateAwaitCancelledFuture)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/system" ...
        val fut: Future[Int] = Future[Int]{}
        fut.Cancel()
        Await(fut)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        StatusRef(lyric_common::SymbolPath({"Cancelled"})))));
}

TEST_F(StdSystemSystem, EvaluateAwaitDefaultForCompletedFuture)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/system" ...
        val fut: Future[Int] = Future[Int]{}
        fut.Complete(42)
        AwaitOrDefault(fut, 0)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(42))));
}

TEST_F(StdSystemSystem, EvaluateAwaitDefaultForRejectedFuture)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/system" ...
        val fut: Future[Int] = Future[Int]{}
        val failure: Internal = Internal{message = "internal failure"}
        fut.Reject(failure)
        AwaitOrDefault(fut, 42)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(42))));
}

TEST_F(StdSystemSystem, EvaluateAwaitDefaultForCancelledFuture)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/system" ...
        val fut: Future[Int] = Future[Int]{}
        fut.Cancel()
        AwaitOrDefault(fut, 42)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(42))));
}

TEST_F(StdSystemSystem, EvaluateAwaitSleep)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/system" ...
        val fut: Future[Nil] = Sleep(1000)
        Await(fut)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellNil())));
}

TEST_F(StdSystemSystem, EvaluateAwaitSpawn)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/system" ...

        val x: Function0[Int] = lambda (): Int {
            Await(Sleep(1000))
            42
        }

        val fut: Future[Nil] = Spawn(x)
        Await(fut)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(42))));
}
