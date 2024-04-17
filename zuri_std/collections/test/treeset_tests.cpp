#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_config/workspace_config.h>
#include <zuri_std_collections/config.h>

class StdTreeSet : public ::testing::Test {
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

TEST_F(StdTreeSet, TestEvaluateNewTreeSet)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: TreeSet[Int] = TreeSet[Int]{}
        ints
    )", options);

    ASSERT_THAT (result.getResult(), ContainsResult(
        RunModule(Return(IsRefType(
        lyric_common::SymbolUrl(
            lyric_common::AssemblyLocation::fromString(ZURI_STD_COLLECTIONS_LOCATION),
            lyric_common::SymbolPath({"TreeSet"})))))));
}

TEST_F(StdTreeSet, TestEvaluateTreeSetSize)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: TreeSet[Int] = TreeSet[Int]{1, 2, 3}
        ints.Size()
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(3LL)))));
}

TEST_F(StdTreeSet, TestEvaluateTreeSetContains)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: TreeSet[Int] = TreeSet[Int]{1, 2, 3}
        ints.Contains(2)
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellBool(true)))));
}

TEST_F(StdTreeSet, TestEvaluateTreeSetAdd)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: TreeSet[Int] = TreeSet[Int]{1, 2, 3}
        ints.Add(42)
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellBool(true)))));
}

TEST_F(StdTreeSet, TestEvaluateTreeSetRemove)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: TreeSet[Int] = TreeSet[Int]{1, 2, 3}
        ints.Remove(3)
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellBool(true)))));
}

TEST_F(StdTreeSet, TestEvaluateTreeSetClear)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: TreeSet[Int] = TreeSet[Int]{1, 2, 3}
        ints.Clear()
        ints.Size()
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(0LL)))));
}

TEST_F(StdTreeSet, TestEvaluateTreeSetIterate)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: TreeSet[Int] = TreeSet[Int]{1, 2, 3}
        var sum: Int = 0
        for n: Int in ints {
            set sum += n
        }
        sum
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(6LL)))));
}