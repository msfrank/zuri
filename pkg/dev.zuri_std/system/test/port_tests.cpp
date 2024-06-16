#include <gtest/gtest.h>

#include <lyric_test/lyric_protocol_tester.h>
#include <lyric_test/mock_receive.h>
#include <lyric_test/mock_send.h>
#include <lyric_serde/patchset_change.h>
#include <lyric_serde/patchset_state.h>
#include <lyric_serde/patchset_value.h>
#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_config/workspace_config.h>
#include <tempo_test/tempo_test.h>

class StdSystemPort : public ::testing::Test {
protected:
    lyric_test::TesterOptions testerOptions;
    lyric_test::ProtocolTesterOptions protocolTesterOptions;

    void SetUp() override {
        auto loadWorkspaceResult = tempo_config::WorkspaceConfig::load(
            TESTER_CONFIG_PATH, {});
        ASSERT_TRUE (loadWorkspaceResult.isResult());
        auto config = loadWorkspaceResult.getResult();

        testerOptions.buildConfig = config->getToolConfig();
        testerOptions.buildVendorConfig = config->getVendorConfig();
        protocolTesterOptions.buildConfig = config->getToolConfig();
        protocolTesterOptions.buildVendorConfig = config->getVendorConfig();
    }
};

TEST_F(StdSystemPort, EvaluateSendEmitOperationWithStringValue)
{
    auto mockSend = std::make_shared<lyric_test::MockSend>();
    protocolTesterOptions.protocolMocks[tempo_utils::Url::fromString("dev.zuri.proto:test")] = mockSend;

    auto result = lyric_test::LyricProtocolTester::runSingleModuleInMockSandbox(R"(
        import from "//std/system" ...
        val port: Port = Acquire(`dev.zuri.proto:test`)
        port.Send(EmitOperation{"hello, world!"})
    )", protocolTesterOptions);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));

    auto messages = mockSend->getMessages();
    ASSERT_EQ (messages.size(), 1);
    auto message1 = messages.at(0).getPatchset();
    ASSERT_EQ (message1.numChanges(), 1);
    auto change1 = message1.getChange(0);
    ASSERT_EQ (change1.getOperationType(), lyric_serde::ChangeOperation::EmitOperation);
    auto emit = change1.getEmitOperation();
    auto value = emit.getValue();
    ASSERT_TRUE (value.isValid());
    ASSERT_EQ (value.getValueType(), lyric_serde::ValueType::String);
    ASSERT_EQ (std::string("hello, world!"), value.getString());
}

TEST_F(StdSystemPort, EvaluateReceiveEmitOperationWithStringValue)
{
    lyric_serde::PatchsetState state;
    auto *change = state.appendChange("").orElseThrow();
    auto *value = state.appendValue(tempo_utils::AttrValue("hello, world!")).orElseThrow();
    change->setEmitOperation(value->getAddress());
    auto toPatchsetResult = state.toPatchset();
    ASSERT_TRUE (toPatchsetResult.isResult());
    std::vector<lyric_serde::LyricPatchset> messages{toPatchsetResult.getResult()};
    auto mockReceive = std::make_shared<lyric_test::MockReceive>(messages);

    protocolTesterOptions.protocolMocks[tempo_utils::Url::fromString("dev.zuri.proto:test")] = mockReceive;

    auto result = lyric_test::LyricProtocolTester::runSingleModuleInMockSandbox(R"(
        import from "//std/system" ...
        val port: Port = Acquire(`dev.zuri.proto:test`)
        val fut: Future[Operation] = port.Receive()
        match Await(fut) {
          case emit: EmitOperation
            match emit.value {
              case x: String
                x == "hello, world!"
              else false
            }
          else false
        }
    )", protocolTesterOptions);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}

TEST_F(StdSystemPort, EvaluateSendEmitOperationWithAttrValue)
{
    auto mockSend = std::make_shared<lyric_test::MockSend>();
    protocolTesterOptions.protocolMocks[tempo_utils::Url::fromString("dev.zuri.proto:test")] = mockSend;

    auto result = lyric_test::LyricProtocolTester::runSingleModuleInMockSandbox(R"(
        import from "//std/system" ...
        val port: Port = Acquire(`dev.zuri.proto:test`)
        val root: Attr = Attr{`io.fathomdata:ns:richtext-1`, 29, "Hello, world"}
        port.Send(EmitOperation{root})
    )", protocolTesterOptions);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));

    auto messages = mockSend->getMessages();
    ASSERT_EQ (messages.size(), 1);
    auto message1 = messages.at(0).getPatchset();
    ASSERT_EQ (message1.numChanges(), 1);
    auto change1 = message1.getChange(0);
    ASSERT_EQ (change1.getOperationType(), lyric_serde::ChangeOperation::EmitOperation);
    auto emit = change1.getEmitOperation();
    auto value = emit.getValue();
    ASSERT_TRUE (value.isValid());
    ASSERT_EQ (value.getValueType(), lyric_serde::ValueType::Attr);
    ASSERT_EQ (value.getAttrNamespace().urlView(), "io.fathomdata:ns:richtext-1");
    ASSERT_EQ (value.getAttrResource().idValue, 29);
    auto child1 = value.getAttrValue();
    ASSERT_EQ (child1.getValueType(), lyric_serde::ValueType::String);
    ASSERT_EQ (child1.getString(), std::string("Hello, world"));
}

TEST_F(StdSystemPort, EvaluateSendEmitOperationWithElementValue)
{
    auto mockSend = std::make_shared<lyric_test::MockSend>();
    protocolTesterOptions.protocolMocks[tempo_utils::Url::fromString("dev.zuri.proto:test")] = mockSend;

    auto result = lyric_test::LyricProtocolTester::runSingleModuleInMockSandbox(R"(
        import from "//std/system" ...
        val port: Port = Acquire(`dev.zuri.proto:test`)
        val root: Element = Element{`io.fathomdata:ns:richtext-1`, 29, "Hello, world"}
        port.Send(EmitOperation{root})
    )", protocolTesterOptions);

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));

    auto messages = mockSend->getMessages();
    ASSERT_EQ (messages.size(), 1);
    auto message1 = messages.at(0).getPatchset();
    ASSERT_EQ (message1.numChanges(), 1);
    auto change1 = message1.getChange(0);
    ASSERT_EQ (change1.getOperationType(), lyric_serde::ChangeOperation::EmitOperation);
    auto emit = change1.getEmitOperation();
    auto value = emit.getValue();
    ASSERT_TRUE (value.isValid());
    ASSERT_EQ (value.getValueType(), lyric_serde::ValueType::Element);
    ASSERT_EQ (value.getElementNamespace().urlView(), "io.fathomdata:ns:richtext-1");
    ASSERT_EQ (value.getElementResource().idValue, 29);
    ASSERT_EQ (value.numElementChildren(), 1);
    auto child1 = value.getElementChild(0);
    ASSERT_EQ (child1.getValueType(), lyric_serde::ValueType::String);
    ASSERT_EQ (child1.getString(), std::string("Hello, world"));
}
