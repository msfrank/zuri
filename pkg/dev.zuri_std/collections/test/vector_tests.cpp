#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_config/workspace_config.h>
#include <tempo_test/tempo_test.h>
#include <zuri_std_collections/config.h>

class StdVector : public ::testing::Test {
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

TEST_F(StdVector, TestEvaluateNewVector)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: Vector[Int] = Vector[Int]{}
        ints
    )", options);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        DataCellRef(
            lyric_common::SymbolUrl(
                lyric_common::AssemblyLocation::fromString(ZURI_STD_COLLECTIONS_LOCATION),
                lyric_common::SymbolPath({"Vector"}))))));
}

TEST_F(StdVector, TestEvaluateVectorSize)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: Vector[Int] = Vector[Int]{1, 2, 3}
        ints.Size()
    )", options);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(3LL))));
}

TEST_F(StdVector, TestEvaluateVectorAt)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: Vector[Int] = Vector[Int]{1, 2, 3}
        ints.At(0)
    )", options);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(1LL))));
}

TEST_F(StdVector, TestEvaluateVectorInsert)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: Vector[Int] = Vector[Int]{1, 2, 3}
        ints.Insert(1, 42)
        ints.At(1)
    )", options);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(42LL))));
}

TEST_F(StdVector, TestEvaluateVectorAppend)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: Vector[Int] = Vector[Int]{1, 2, 3}
        ints.Append(4)
        ints.At(3)
    )", options);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(4LL))));
}

TEST_F(StdVector, TestEvaluateVectorReplace)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: Vector[Int] = Vector[Int]{1, 2, 3}
        ints.Replace(2, 42)
    )", options);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(3LL))));
}

TEST_F(StdVector, TestEvaluateVectorRemove)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: Vector[Int] = Vector[Int]{1, 2, 3}
        ints.Remove(0)
    )", options);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(1LL))));
}

TEST_F(StdVector, TestEvaluateVectorClear)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: Vector[Int] = Vector[Int]{1, 2, 3}
        ints.Clear()
        ints.Size()
    )", options);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(0LL))));
}

TEST_F(StdVector, TestEvaluateVectorIterate)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: Vector[Int] = Vector[Int]{1, 2, 3}
        var sum: Int = 0
        for n: Int in ints {
            set sum += n
        }
        sum
    )", options);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(6LL))));
}