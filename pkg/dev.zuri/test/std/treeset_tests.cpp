#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>
#include <zuri_test/zuri_tester.h>
#include <zuri_test_runtime/zuri_test_runtime.h>

class StdCollectionsTreeSet : public ::testing::Test {
protected:
    std::unique_ptr<zuri_test::ZuriTester> tester;

    void SetUp() override {
        auto runtime = zuri_test_runtime::get_global_test_runtime();
        zuri_test::TesterOptions options;
        options.localPackages.emplace_back(ZURI_STD_PACKAGE_PATH);
        tester = std::make_unique<zuri_test::ZuriTester>(runtime, options);
        TU_RAISE_IF_NOT_OK (tester->configure());
    }
};

TEST_F(StdCollectionsTreeSet, TestEvaluateNewTreeSet)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: TreeSet[I64] = TreeSet[I64]{}
        ints
    )");

    ASSERT_THAT (result.getResult(), tempo_test::ContainsResult(RunModule(
        OperandRef(
            lyric_common::SymbolUrl(
                lyric_common::ModuleLocation::fromString("dev.zuri.pkg://std-0.0.1@zuri.dev/collections"),
                lyric_common::SymbolPath({"TreeSet"}))))));
}

TEST_F(StdCollectionsTreeSet, TestEvaluateConstructTreeSetWithElements)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: TreeSet[I64] = TreeSet[I64]{1, 2, 3}
        ints.Size()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        OperandI64(3))));
}

TEST_F(StdCollectionsTreeSet, TestEvaluateTreeSetSize)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: TreeSet[I64] = TreeSet[I64]{}
        ints.Size()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(OperandI64(0LL))));
}

TEST_F(StdCollectionsTreeSet, TestEvaluateTreeSetContains)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: TreeSet[I64] = TreeSet[I64]{}
        ints.Add(1)
        ints.Add(2)
        ints.Add(3)
        ints.Contains(2)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(OperandBool(true))));
}

TEST_F(StdCollectionsTreeSet, TestEvaluateTreeSetAdd)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: TreeSet[I64] = TreeSet[I64]{}
        ints.Add(42)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(OperandBool(true))));
}

TEST_F(StdCollectionsTreeSet, TestEvaluateTreeSetRemove)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: TreeSet[I64] = TreeSet[I64]{}
        ints.Add(1)
        ints.Add(2)
        ints.Add(3)
        ints.Remove(3)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(OperandBool(true))));
}

TEST_F(StdCollectionsTreeSet, TestEvaluateTreeSetClear)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: TreeSet[I64] = TreeSet[I64]{}
        ints.Add(1)
        ints.Add(2)
        ints.Add(3)
        ints.Clear()
        ints.Size()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(OperandI64(0LL))));
}

TEST_F(StdCollectionsTreeSet, TestEvaluateTreeSetIterate)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: TreeSet[I64] = TreeSet[I64]{}
        ints.Add(1)
        ints.Add(2)
        ints.Add(3)
        var sum: I64 = 0
        for n: I64 in ints {
            sum += n
        }
        sum
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(OperandI64(6LL))));
}