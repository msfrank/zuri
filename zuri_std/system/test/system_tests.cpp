#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_config/workspace_config.h>

class StdSystemSystem : public ::testing::Test {
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

TEST_F(StdSystemSystem, EvaluateAwaitCompletedFuture)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/system" ...
        val fut: Future[Int] = Future[Int]{}
        fut.Complete(42)
        Await(fut)
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(42)))));
}

TEST_F(StdSystemSystem, EvaluateAwaitRejectedFuture)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/system" ...
        val fut: Future[Int] = Future[Int]{}
        val failure: Internal = Internal{message = "internal failure"}
        fut.Reject(failure)
        Await(fut)
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(IsRefType(lyric_common::SymbolPath({"Internal"}))))));
}

TEST_F(StdSystemSystem, EvaluateAwaitCancelledFuture)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/system" ...
        val fut: Future[Int] = Future[Int]{}
        fut.Cancel()
        Await(fut)
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(IsRefType(lyric_common::SymbolPath({"Cancelled"}))))));
}

TEST_F(StdSystemSystem, EvaluateAwaitDefaultForCompletedFuture)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/system" ...
        val fut: Future[Int] = Future[Int]{}
        fut.Complete(42)
        AwaitOrDefault(fut, 0)
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(42)))));
}

TEST_F(StdSystemSystem, EvaluateAwaitDefaultForRejectedFuture)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/system" ...
        val fut: Future[Int] = Future[Int]{}
        val failure: Internal = Internal{message = "internal failure"}
        fut.Reject(failure)
        AwaitOrDefault(fut, 42)
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(42)))));
}

TEST_F(StdSystemSystem, EvaluateAwaitDefaultForCancelledFuture)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/system" ...
        val fut: Future[Int] = Future[Int]{}
        fut.Cancel()
        AwaitOrDefault(fut, 42)
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(42)))));
}

TEST_F(StdSystemSystem, EvaluateAwaitSleep)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/system" ...
        val fut: Future[Nil] = Sleep(1000)
        Await(fut)
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellNil()))));
}

TEST_F(StdSystemSystem, EvaluateAwaitSpawn)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/system" ...

        val x: Function0[Int] = lambda (): Int {
            Await(Sleep(1000))
            42
        }

        val fut: Future[Nil] = Spawn(x)
        Await(fut)
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(42)))));
}
