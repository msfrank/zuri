#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_config/workspace_config.h>
#include <zuri_std_collections/config.h>

class StdTreeMap : public ::testing::Test {
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

TEST_F(StdTreeMap, TestEvaluateNewMap)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: TreeMap[Int,Int] = TreeMap[Int,Int]{}
        ints
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(IsRefType(
            lyric_common::SymbolUrl(
                lyric_common::AssemblyLocation::fromString(ZURI_STD_COLLECTIONS_LOCATION),
                lyric_common::SymbolPath({"TreeMap"})))))));
}

TEST_F(StdTreeMap, TestEvaluateTreeMapConstruction)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: TreeMap[Int,Int] = TreeMap[Int,Int]{
            Tuple2[Int,Int]{1, 11},
            Tuple2[Int,Int]{2, 12},
            Tuple2[Int,Int]{3, 13}
        }
        ints.size()
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(3LL)))));
}

TEST_F(StdTreeMap, TestEvaluateMapSize)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: TreeMap[Int,Int] = TreeMap[Int,Int]{}
        ints.put(1, 11)
        ints.put(2, 12)
        ints.put(3, 13)
        ints.size()
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(3LL)))));
}

TEST_F(StdTreeMap, TestEvaluateMapPutAndContains)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: TreeMap[Int,Int] = TreeMap[Int,Int]{}
        ints.put(1, 11)
        ints.put(2, 12)
        ints.put(3, 13)
        ints.contains(2)
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellBool(true)))));
}

TEST_F(StdTreeMap, TestEvaluateMapPutAndGet)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: TreeMap[Int,Int] = TreeMap[Int,Int]{}
        ints.put(1, 11)
        ints.put(2, 12)
        ints.put(3, 13)
        ints.get(2)
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(12LL)))));
}

TEST_F(StdTreeMap, TestEvaluateMapPutAndRemove)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: TreeMap[Int,Int] = TreeMap[Int,Int]{}
        ints.put(1, 11)
        ints.put(2, 12)
        ints.put(3, 13)
        ints.remove(2)
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(12LL)))));
}

TEST_F(StdTreeMap, TestEvaluateMapPutAndClear)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: TreeMap[Int,Int] = TreeMap[Int,Int]{}
        ints.put(1, 11)
        ints.put(2, 12)
        ints.put(3, 13)
        ints.clear()
        ints.size()
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(0LL)))));
}