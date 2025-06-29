#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_config/workspace_config.h>
#include <tempo_test/tempo_test.h>
#include <zuri_test/zuri_tester.h>

class StdCollectionsTreeSet : public ::testing::Test {
protected:
    std::unique_ptr<zuri_test::ZuriTester> tester;

    void SetUp() override {
        zuri_test::TesterOptions options;
        options.localPackages.emplace_back(ZURI_STD_PACKAGE_PATH);
        tester = std::make_unique<zuri_test::ZuriTester>(options);
        TU_RAISE_IF_NOT_OK (tester->configure());
    }
};

TEST_F(StdCollectionsTreeSet, TestEvaluateNewTreeSet)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: TreeSet[Int] = TreeSet[Int]{}
        ints
    )");

    ASSERT_THAT (result.getResult(), tempo_test::ContainsResult(RunModule(
        DataCellRef(
            lyric_common::SymbolUrl(
                lyric_common::ModuleLocation::fromString("dev.zuri.pkg://std-0.0.1@zuri.dev/collections"),
                lyric_common::SymbolPath({"TreeSet"}))))));
}

TEST_F(StdCollectionsTreeSet, TestEvaluateTreeSetSize)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: TreeSet[Int] = TreeSet[Int]{}
        ints.Size()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellInt(0LL))));
}

TEST_F(StdCollectionsTreeSet, TestEvaluateTreeSetContains)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: TreeSet[Int] = TreeSet[Int]{}
        ints.Add(1)
        ints.Add(2)
        ints.Add(3)
        ints.Contains(2)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellBool(true))));
}

TEST_F(StdCollectionsTreeSet, TestEvaluateTreeSetAdd)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: TreeSet[Int] = TreeSet[Int]{}
        ints.Add(42)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellBool(true))));
}

TEST_F(StdCollectionsTreeSet, TestEvaluateTreeSetRemove)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: TreeSet[Int] = TreeSet[Int]{}
        ints.Add(1)
        ints.Add(2)
        ints.Add(3)
        ints.Remove(3)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellBool(true))));
}

TEST_F(StdCollectionsTreeSet, TestEvaluateTreeSetClear)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: TreeSet[Int] = TreeSet[Int]{}
        ints.Add(1)
        ints.Add(2)
        ints.Add(3)
        ints.Clear()
        ints.Size()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellInt(0LL))));
}

TEST_F(StdCollectionsTreeSet, TestEvaluateTreeSetIterate)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: TreeSet[Int] = TreeSet[Int]{}
        ints.Add(1)
        ints.Add(2)
        ints.Add(3)
        var sum: Int = 0
        for n: Int in ints {
            set sum += n
        }
        sum
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellInt(6LL))));
}