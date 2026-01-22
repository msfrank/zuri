#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>
#include <zuri_test/zuri_tester.h>

#include "test_utils.h"

class StdCollectionsOption : public ::testing::Test {
protected:
    std::unique_ptr<zuri_test::ZuriTester> tester;

    void SetUp() override {
        auto runtime = get_global_test_runtime();
        zuri_test::TesterOptions options;
        options.localPackages.emplace_back(ZURI_STD_PACKAGE_PATH);
        tester = std::make_unique<zuri_test::ZuriTester>(runtime, options);
        TU_RAISE_IF_NOT_OK (tester->configure());
    }
};

TEST_F(StdCollectionsOption, TestEvaluateNewEmptyOption)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val opt: Option[Int] = Option[Int]{}
        opt
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        DataCellRef(
            lyric_common::SymbolUrl(
                lyric_common::ModuleLocation::fromString("dev.zuri.pkg://std-0.0.1@zuri.dev/collections"),
                lyric_common::SymbolPath({"Option"}))))));
}

TEST_F(StdCollectionsOption, TestEvaluateNewOption)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val opt: Option[Int] = Option[Int]{42}
        opt
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        DataCellRef(
            lyric_common::SymbolUrl(
                lyric_common::ModuleLocation::fromString("dev.zuri.pkg://std-0.0.1@zuri.dev/collections"),
                lyric_common::SymbolPath({"Option"}))))));
}

TEST_F(StdCollectionsOption, TestEvaluateOptionIsEmpty)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val opt: Option[Int] = Option[Int]{}
        opt.IsEmpty()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellBool(true))));
}

TEST_F(StdCollectionsOption, TestEvaluateOptionIsNotEmpty)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val opt: Option[Int] = Option[Int]{42}
        opt.IsEmpty()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellBool(false))));
}

TEST_F(StdCollectionsOption, TestEvaluateOptionGetWhenEmpty)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val opt: Option[Int] = Option[Int]{}
        opt.Get()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellNil())));
}

TEST_F(StdCollectionsOption, TestEvaluateOptionGetWhenNotEmpty)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val opt: Option[Int] = Option[Int]{42}
        opt.Get()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellInt(42))));
}

TEST_F(StdCollectionsOption, TestEvaluateOptionGetOrElseWhenEmpty)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val opt: Option[Int] = Option[Int]{}
        opt.GetOrElse(0)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellInt(0))));
}

TEST_F(StdCollectionsOption, TestEvaluateOptionGetOrElseWhenNotEmpty)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/collections" ...
        val opt: Option[Int] = Option[Int]{42}
        opt.GetOrElse(0)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellInt(42))));
}
