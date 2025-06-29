#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_config/workspace_config.h>
#include <tempo_test/tempo_test.h>
#include <zuri_test/zuri_tester.h>

class StdCollectionsVector : public ::testing::Test {
protected:
    std::unique_ptr<zuri_test::ZuriTester> tester;

    void SetUp() override {
        zuri_test::TesterOptions options;
        options.localPackages.emplace_back(ZURI_STD_PACKAGE_PATH);
        tester = std::make_unique<zuri_test::ZuriTester>(options);
        TU_RAISE_IF_NOT_OK (tester->configure());
    }
};

TEST_F(StdCollectionsVector, TestEvaluateNewVector)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: Vector[Int] = Vector[Int]{}
        ints
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        DataCellRef(
            lyric_common::SymbolUrl(
                lyric_common::ModuleLocation::fromString("dev.zuri.pkg://std-0.0.1@zuri.dev/collections"),
                lyric_common::SymbolPath({"Vector"}))))));
}

TEST_F(StdCollectionsVector, TestEvaluateVectorSize)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: Vector[Int] = Vector[Int]{}
        ints.Size()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellInt(0LL))));
}

TEST_F(StdCollectionsVector, TestEvaluateVectorAt)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: Vector[Int] = Vector[Int]{}
        ints.Append(1)
        ints.At(0)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellInt(1LL))));
}

TEST_F(StdCollectionsVector, TestEvaluateVectorInsert)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: Vector[Int] = Vector[Int]{}
        ints.Append(1)
        ints.Append(2)
        ints.Append(3)
        ints.Insert(1, 42)
        ints.At(1)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellInt(42LL))));
}

TEST_F(StdCollectionsVector, TestEvaluateVectorAppend)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: Vector[Int] = Vector[Int]{}
        ints.Append(1)
        ints.Append(2)
        ints.At(1)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellInt(2LL))));
}

TEST_F(StdCollectionsVector, TestEvaluateVectorReplace)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: Vector[Int] = Vector[Int]{}
        ints.Append(1)
        ints.Replace(0, 42)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellInt(1LL))));
}

TEST_F(StdCollectionsVector, TestEvaluateVectorRemove)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: Vector[Int] = Vector[Int]{}
        ints.Append(1)
        ints.Remove(0)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellInt(1LL))));
}

TEST_F(StdCollectionsVector, TestEvaluateVectorClear)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: Vector[Int] = Vector[Int]{}
        ints.Append(1)
        ints.Append(2)
        ints.Append(3)
        ints.Clear()
        ints.Size()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellInt(0LL))));
}

TEST_F(StdCollectionsVector, TestEvaluateVectorIterate)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: Vector[Int] = Vector[Int]{}
        ints.Append(1)
        ints.Append(2)
        ints.Append(3)
        var sum: Int = 0
        for n: Int in ints {
            set sum += n
        }
        sum
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellInt(6LL))));
}