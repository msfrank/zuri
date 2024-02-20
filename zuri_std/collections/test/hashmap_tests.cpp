#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_config/workspace_config.h>
#include <zuri_std_collections/config.h>

class StdHashMap : public ::testing::Test {
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

TEST_F(StdHashMap, TestEvaluateNewHashMap)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: HashMap[Int,Int] = HashMap[Int,Int]{}
        ints
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(IsRefType(
            lyric_common::SymbolUrl(
                lyric_common::AssemblyLocation::fromString(ZURI_STD_COLLECTIONS_LOCATION),
                lyric_common::SymbolPath({"HashMap"})))))));
}

TEST_F(StdHashMap, TestEvaluateHashMapConstruction)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: HashMap[Int,Int] = HashMap[Int,Int]{
            Tuple2[Int,Int]{1, 11},
            Tuple2[Int,Int]{2, 12},
            Tuple2[Int,Int]{3, 13}
        }
        ints.size()
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(3LL)))));
}

TEST_F(StdHashMap, TestEvaluateHashMapSize)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: HashMap[Int,Int] = HashMap[Int,Int]{}
        ints.put(1, 11)
        ints.put(2, 12)
        ints.put(3, 13)
        ints.size()
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(3LL)))));
}

TEST_F(StdHashMap, TestEvaluateHashMapPutAndContains)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: HashMap[Int,Int] = HashMap[Int,Int]{}
        ints.put(1, 11)
        ints.put(2, 12)
        ints.put(3, 13)
        ints.contains(2)
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellBool(true)))));
}

TEST_F(StdHashMap, TestEvaluateHashMapPutAndGet)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: HashMap[Int,Int] = HashMap[Int,Int]{}
        ints.put(1, 11)
        ints.put(2, 12)
        ints.put(3, 13)
        ints.get(2)
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(12LL)))));
}

TEST_F(StdHashMap, TestEvaluateHashMapPutAndRemove)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: HashMap[Int,Int] = HashMap[Int,Int]{}
        ints.put(1, 11)
        ints.put(2, 12)
        ints.put(3, 13)
        ints.remove(2)
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(12LL)))));
}

TEST_F(StdHashMap, TestEvaluateHashMapPutAndClear)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: HashMap[Int,Int] = HashMap[Int,Int]{}
        ints.put(1, 11)
        ints.put(2, 12)
        ints.put(3, 13)
        ints.clear()
        ints.size()
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(0LL)))));
}