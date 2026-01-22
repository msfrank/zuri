#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <lyric_test/mock_send.h>
#include <tempo_test/tempo_test.h>
#include <zuri_test/zuri_tester.h>

#include "test_utils.h"

class StdLogLog : public ::testing::Test {
protected:
    std::shared_ptr<lyric_test::MockSend> mockSend;
    std::unique_ptr<zuri_test::ZuriTester> tester;

    void SetUp() override {
        auto runtime = get_global_test_runtime();
        zuri_test::TesterOptions options;
        options.localPackages.emplace_back(ZURI_STD_PACKAGE_PATH);
        mockSend = std::make_shared<lyric_test::MockSend>();
        options.protocolMocks[tempo_utils::Url::fromString("dev.zuri.proto:log")] = mockSend;
        tester = std::make_unique<zuri_test::ZuriTester>(runtime, options);
        TU_RAISE_IF_NOT_OK (tester->configure());
    }
};

TEST_F(StdLogLog, EvaluateLog)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/log" ...
        Log("hello world!")
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));

    auto messages = mockSend->getMessages();
    ASSERT_EQ (messages.size(), 1);
    auto &message1 = messages.at(0);
    std::string_view payload((const char *) message1->getData(), message1->getSize());
    ASSERT_EQ (std::string("hello world!"), payload);
}