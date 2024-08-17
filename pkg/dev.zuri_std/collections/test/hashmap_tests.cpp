#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_config/workspace_config.h>
#include <tempo_test/tempo_test.h>
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

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        DataCellRef(
            lyric_common::SymbolUrl(
                lyric_common::ModuleLocation::fromString(ZURI_STD_COLLECTIONS_LOCATION),
                lyric_common::SymbolPath({"HashMap"}))))));
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
        ints.Size()
    )", options);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(3LL))));
}

TEST_F(StdHashMap, TestEvaluateHashMapSize)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: HashMap[Int,Int] = HashMap[Int,Int]{}
        ints.Put(1, 11)
        ints.Put(2, 12)
        ints.Put(3, 13)
        ints.Size()
    )", options);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(3LL))));
}

TEST_F(StdHashMap, TestEvaluateHashMapPutAndContains)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: HashMap[Int,Int] = HashMap[Int,Int]{}
        ints.Put(1, 11)
        ints.Put(2, 12)
        ints.Put(3, 13)
        ints.Contains(2)
    )", options);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}

TEST_F(StdHashMap, TestEvaluateHashMapPutAndGet)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: HashMap[Int,Int] = HashMap[Int,Int]{}
        ints.Put(1, 11)
        ints.Put(2, 12)
        ints.Put(3, 13)
        ints.Get(2)
    )", options);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(12LL))));
}

TEST_F(StdHashMap, TestEvaluateHashMapPutAndRemove)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: HashMap[Int,Int] = HashMap[Int,Int]{}
        ints.Put(1, 11)
        ints.Put(2, 12)
        ints.Put(3, 13)
        ints.Remove(2)
    )", options);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(12LL))));
}

TEST_F(StdHashMap, TestEvaluateHashMapPutAndClear)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: HashMap[Int,Int] = HashMap[Int,Int]{}
        ints.Put(1, 11)
        ints.Put(2, 12)
        ints.Put(3, 13)
        ints.Clear()
        ints.Size()
    )", options);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(0LL))));
}