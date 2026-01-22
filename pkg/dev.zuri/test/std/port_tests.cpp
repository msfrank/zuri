#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <lyric_test/mock_send.h>
#include <tempo_test/tempo_test.h>
#include <zuri_test/zuri_tester.h>

#include "test_utils.h"

class StdSystemPort : public ::testing::Test {
protected:
    std::unique_ptr<zuri_test::ZuriTester> tester;
    std::shared_ptr<lyric_test::MockSend> mockSend;

    void SetUp() override {
        auto runtime = get_global_test_runtime();
        zuri_test::TesterOptions options;
        mockSend = std::make_shared<lyric_test::MockSend>();
        options.protocolMocks[tempo_utils::Url::fromString("dev.zuri.proto:test")] = mockSend;
        options.localPackages.emplace_back(ZURI_STD_PACKAGE_PATH);
        tester = std::make_unique<zuri_test::ZuriTester>(runtime, options);
        TU_RAISE_IF_NOT_OK (tester->configure());
    }
};

TEST_F(StdSystemPort, EvaluateSendEmitOperationWithStringValue)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/system" ...
        val port: Port = Acquire(`dev.zuri.proto:test`)
        port.Send("hello, world!".ToBytes())
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));

    auto messages = mockSend->getMessages();
    ASSERT_EQ (messages.size(), 1);
    auto &message1 = messages.at(0);
    std::string_view payload((const char *) message1->getData(), message1->getSize());
    ASSERT_EQ (std::string("hello, world!"), payload);
}