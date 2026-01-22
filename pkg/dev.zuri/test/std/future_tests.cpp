#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>
#include <zuri_test/zuri_tester.h>

#include "test_utils.h"

class StdSystemFuture : public ::testing::Test {
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

TEST_F(StdSystemFuture, EvaluateAwaitComposedFuture)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/system" ...
        val fut1: Future[Int] = Future[Int]{}
        val fut2: Future[Int] = fut1.Then[Function1[Int|Status,Int]](lambda(i: Int): Int|Status {
            i + 1
        })
        fut1.Complete(42)
        Await(fut2)
    )");

    ASSERT_THAT (result,
        tempo_test::ContainsResult(RunModule(DataCellInt(43))));
}
