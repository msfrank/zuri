#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_config/workspace_config.h>
#include <tempo_test/tempo_test.h>
#include <zuri_test/zuri_tester.h>

class StdCollectionsTreeMap : public ::testing::Test {
protected:
    std::unique_ptr<zuri_test::ZuriTester> tester;

    void SetUp() override {
        zuri_test::TesterOptions options;
        options.localPackages.emplace_back(ZURI_STD_PACKAGE_PATH);
        tester = std::make_unique<zuri_test::ZuriTester>(options);
        TU_RAISE_IF_NOT_OK (tester->configure());
    }
};

TEST_F(StdCollectionsTreeMap, TestEvaluateNewMap)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: TreeMap[Int,Int] = TreeMap[Int,Int]{}
        ints
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        DataCellRef(
            lyric_common::SymbolUrl(
                lyric_common::ModuleLocation::fromString("dev.zuri.pkg://std-0.0.1@zuri.dev/collections"),
                lyric_common::SymbolPath({"TreeMap"}))))));
}

TEST_F(StdCollectionsTreeMap, TestEvaluateConstructTreeMapWithEntries)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: TreeMap[Int,Int] = TreeMap[Int,Int]{
            Tuple2[Int,Int]{1, 11},
            Tuple2[Int,Int]{2, 12},
            Tuple2[Int,Int]{3, 13}
        }
        ints.Size()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellInt(3))));
}

TEST_F(StdCollectionsTreeMap, TestEvaluateMapSize)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: TreeMap[Int,Int] = TreeMap[Int,Int]{}
        ints.Put(1, 11)
        ints.Put(2, 12)
        ints.Put(3, 13)
        ints.Size()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellInt(3LL))));
}

TEST_F(StdCollectionsTreeMap, TestEvaluateMapPutAndContains)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: TreeMap[Int,Int] = TreeMap[Int,Int]{}
        ints.Put(1, 11)
        ints.Put(2, 12)
        ints.Put(3, 13)
        ints.Contains(2)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellBool(true))));
}

TEST_F(StdCollectionsTreeMap, TestEvaluateMapPutAndGet)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: TreeMap[Int,Int] = TreeMap[Int,Int]{}
        ints.Put(1, 11)
        ints.Put(2, 12)
        ints.Put(3, 13)
        ints.Get(2)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellInt(12LL))));
}

TEST_F(StdCollectionsTreeMap, TestEvaluateMapPutAndRemove)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: TreeMap[Int,Int] = TreeMap[Int,Int]{}
        ints.Put(1, 11)
        ints.Put(2, 12)
        ints.Put(3, 13)
        ints.Remove(2)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellInt(12LL))));
}

TEST_F(StdCollectionsTreeMap, TestEvaluateMapPutAndClear)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: TreeMap[Int,Int] = TreeMap[Int,Int]{}
        ints.Put(1, 11)
        ints.Put(2, 12)
        ints.Put(3, 13)
        ints.Clear()
        ints.Size()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellInt(0LL))));
}

TEST_F(StdCollectionsTreeMap, TestEvaluateTreeMapIterate)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: TreeMap[Int,Int] = TreeMap[Int,Int]{
            Tuple2[Int,Int]{1, 11},
            Tuple2[Int,Int]{2, 12},
            Tuple2[Int,Int]{3, 13}
        }
        var sum: Int = 0
        for entry: Tuple2[Int,Int] in ints {
            set sum += entry.Element1
        }
        sum
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellInt(36))));
}
