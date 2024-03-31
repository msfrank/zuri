#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_config/workspace_config.h>
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

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(IsRefType(
        lyric_common::SymbolUrl(
            lyric_common::AssemblyLocation::fromString(ZURI_STD_COLLECTIONS_LOCATION),
            lyric_common::SymbolPath({"Vector"})))))));
}

TEST_F(StdVector, TestEvaluateVectorSize)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: Vector[Int] = Vector[Int]{1, 2, 3}
        ints.size()
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(3LL)))));
}

TEST_F(StdVector, TestEvaluateVectorAt)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: Vector[Int] = Vector[Int]{1, 2, 3}
        ints.at(0)
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(1LL)))));
}

TEST_F(StdVector, TestEvaluateVectorInsert)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: Vector[Int] = Vector[Int]{1, 2, 3}
        ints.insert(1, 42)
        ints.at(1)
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(42LL)))));
}

TEST_F(StdVector, TestEvaluateVectorAppend)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: Vector[Int] = Vector[Int]{1, 2, 3}
        ints.append(4)
        ints.at(3)
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(4LL)))));
}

TEST_F(StdVector, TestEvaluateVectorReplace)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: Vector[Int] = Vector[Int]{1, 2, 3}
        ints.replace(2, 42)
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(3LL)))));
}

TEST_F(StdVector, TestEvaluateVectorRemove)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: Vector[Int] = Vector[Int]{1, 2, 3}
        ints.remove(0)
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(1LL)))));
}

TEST_F(StdVector, TestEvaluateVectorClear)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        import from "//std/collections" ...
        val ints: Vector[Int] = Vector[Int]{1, 2, 3}
        ints.clear()
        ints.size()
    )", options);

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(0LL)))));
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

    ASSERT_THAT (result, ContainsResult(
        RunModule(Return(DataCellInt(6LL)))));
}