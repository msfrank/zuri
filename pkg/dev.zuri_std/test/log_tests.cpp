#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <lyric_test/mock_send.h>
#include <tempo_config/workspace_config.h>
#include <tempo_test/tempo_test.h>
#include <zuri_test/zuri_tester.h>

class StdLogLog : public ::testing::Test {
protected:
    std::shared_ptr<lyric_test::MockSend> mockSend;
    std::unique_ptr<zuri_test::ZuriTester> tester;

    void SetUp() override {
        zuri_test::TesterOptions options;
        options.localPackages.emplace_back(ZURI_STD_PACKAGE_PATH);
        mockSend = std::make_shared<lyric_test::MockSend>();
        options.protocolMocks[tempo_utils::Url::fromString("dev.zuri.proto:log")] = mockSend;
        tester = std::make_unique<zuri_test::ZuriTester>(options);
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
    auto message1 = messages.at(0).getPatchset();
    ASSERT_EQ (message1.numChanges(), 1);
    auto change1 = message1.getChange(0);
    ASSERT_EQ (change1.getOperationType(), lyric_serde::ChangeOperation::EmitOperation);
    auto emit = change1.getEmitOperation();
    auto value = emit.getValue();
    ASSERT_EQ (value.getValueType(), lyric_serde::ValueType::String);
    ASSERT_EQ (std::string("hello world!"), value.getString());
}