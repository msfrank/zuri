#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>
#include <zuri_test/zuri_tester.h>
#include <zuri_test_runtime/zuri_test_runtime.h>

class StdCollectionsTreeMap : public ::testing::Test {
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

TEST_F(StdCollectionsTreeMap, TestEvaluateNewMap)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: TreeMap[I64,I64] = TreeMap[I64,I64]{}
        ints
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        OperandRef(
            lyric_common::SymbolUrl(
                lyric_common::ModuleLocation::fromString("dev.zuri.pkg://std-0.0.1@zuri.dev/collections"),
                lyric_common::SymbolPath({"TreeMap"}))))));
}

TEST_F(StdCollectionsTreeMap, TestEvaluateConstructTreeMapWithEntries)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: TreeMap[I64,I64] = TreeMap[I64,I64]{
            Tuple2[I64,I64]{1, 11},
            Tuple2[I64,I64]{2, 12},
            Tuple2[I64,I64]{3, 13}
        }
        ints.Size()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(OperandI64(3))));
}

TEST_F(StdCollectionsTreeMap, TestEvaluateMapSize)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: TreeMap[I64,I64] = TreeMap[I64,I64]{}
        ints.Put(1, 11)
        ints.Put(2, 12)
        ints.Put(3, 13)
        ints.Size()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(OperandI64(3LL))));
}

TEST_F(StdCollectionsTreeMap, TestEvaluateMapPutAndContains)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: TreeMap[I64,I64] = TreeMap[I64,I64]{}
        ints.Put(1, 11)
        ints.Put(2, 12)
        ints.Put(3, 13)
        ints.Contains(2)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(OperandBool(true))));
}

TEST_F(StdCollectionsTreeMap, TestEvaluateMapPutAndGet)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: TreeMap[I64,I64] = TreeMap[I64,I64]{}
        ints.Put(1, 11)
        ints.Put(2, 12)
        ints.Put(3, 13)
        ints.Get(2)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(OperandI64(12LL))));
}

TEST_F(StdCollectionsTreeMap, TestEvaluateMapPutAndRemove)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: TreeMap[I64,I64] = TreeMap[I64,I64]{}
        ints.Put(1, 11)
        ints.Put(2, 12)
        ints.Put(3, 13)
        ints.Remove(2)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(OperandI64(12LL))));
}

TEST_F(StdCollectionsTreeMap, TestEvaluateMapPutAndClear)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: TreeMap[I64,I64] = TreeMap[I64,I64]{}
        ints.Put(1, 11)
        ints.Put(2, 12)
        ints.Put(3, 13)
        ints.Clear()
        ints.Size()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(OperandI64(0LL))));
}

TEST_F(StdCollectionsTreeMap, TestEvaluateTreeMapIterate)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val ints: TreeMap[I64,I64] = TreeMap[I64,I64]{
            Tuple2[I64,I64]{1, 11},
            Tuple2[I64,I64]{2, 12},
            Tuple2[I64,I64]{3, 13}
        }
        var sum: I64 = 0
        for entry: Tuple2[I64,I64] in ints {
            sum += entry.Element1
        }
        sum
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(OperandI64(36))));
}
