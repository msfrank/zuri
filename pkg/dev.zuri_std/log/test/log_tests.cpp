#include <gtest/gtest.h>

#include <lyric_test/lyric_protocol_tester.h>
#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <lyric_test/mock_send.h>
#include <tempo_config/workspace_config.h>
#include <tempo_test/tempo_test.h>

class StdLogLog : public ::testing::Test {
protected:
    lyric_test::ProtocolTesterOptions baseOptions;

    void SetUp() override {
        auto loadWorkspaceResult = tempo_config::WorkspaceConfig::load(
            TESTER_CONFIG_PATH, {});
        ASSERT_TRUE (loadWorkspaceResult.isResult());
        auto config = loadWorkspaceResult.getResult();
        baseOptions.buildConfig = config->getToolConfig();
        baseOptions.buildVendorConfig = config->getVendorConfig();
    }
};

TEST_F(StdLogLog, EvaluateLog)
{
    lyric_test::ProtocolTesterOptions options = baseOptions;

    auto mockSend = std::make_shared<lyric_test::MockSend>();
    options.protocolMocks[tempo_utils::Url::fromString("dev.zuri.proto:log")] = mockSend;

    auto result = lyric_test::LyricProtocolTester::runSingleModuleInMockSandbox(R"(
        import from "//std/log" ...
        Log("hello world!")
    )", options);

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