#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>
#include <zuri_test/zuri_tester.h>
#include <zuri_test_runtime/zuri_test_runtime.h>

class StdUrl : public ::testing::Test {
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

TEST_F(StdUrl, TestEvaluateNewUrl)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/url" ...
        val url: Url = expect ParseUrl("https://zuri.dev/example/index.html")
        url
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        OperandRef(
            lyric_common::SymbolUrl(
                lyric_common::ModuleLocation::fromString(ZURI_STD_PACKAGE_URL "/url"),
                lyric_common::SymbolPath({"Url"}))))));
}

TEST_F(StdUrl, TestEvaluateNewEmptyUrl)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/url" ...
        val url: Url = Url{}
        url
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        OperandRef(
            lyric_common::SymbolUrl(
                lyric_common::ModuleLocation::fromString(ZURI_STD_PACKAGE_URL "/url"),
                lyric_common::SymbolPath({"Url"}))))));
}

TEST_F(StdUrl, TestEvaluateUrlToString)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/url" ...
        val url: Url = expect ParseUrl("https://zuri.dev/example/index.html")
        url.ToString()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            OperandString("https://zuri.dev/example/index.html"))));

}

TEST_F(StdUrl, TestEvaluateUrlIsEqual)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/url" ...
        val url1: Url = expect ParseUrl("/Hello")
        val url2: Url = expect ParseUrl("/Hello")
        url1 == url2
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(OperandBool(true))));
}