#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>
#include <tempo_utils/memory_bytes.h>
#include <zuri_test_runtime/zuri_test_runtime.h>
#include <zuri_test/test_transport.h>
#include <zuri_test/zuri_tester.h>

class StdStdinProtocol : public ::testing::Test {
protected:
    std::unique_ptr<zuri_test::ZuriTester> tester;
    std::shared_ptr<zuri_test::SingleStreamTestTransport> stdinTransport;

    void SetUp() override {
        auto runtime = zuri_test_runtime::get_global_test_runtime();
        zuri_test::TesterOptions options;
        options.localPackages.emplace_back(ZURI_STD_PACKAGE_PATH);

        stdinTransport = std::make_shared<zuri_test::SingleStreamTestTransport>();
        auto logProtocolUrl = tempo_utils::Url::fromString(ZURI_STD_PACKAGE_URL "/protocol/stdin#StdinProtocol");
        options.localTransports[logProtocolUrl] = stdinTransport;

        lyric_runtime::ConnectorPolicy policy;
        lyric_runtime::PolicyMember anyMember;
        anyMember.type = lyric_runtime::PolicyMember::Type::Any;
        anyMember.member = lyric_runtime::PolicyMember::AnyMember{};
        policy.policyMembers.push_back(anyMember);
        options.protocolConnectors[logProtocolUrl] = policy;

        tester = std::make_unique<zuri_test::ZuriTester>(runtime, options);
        TU_RAISE_IF_NOT_OK (tester->configure());
    }
};

TEST_F(StdStdinProtocol, EvaluateConnect)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/conn" ...
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/protocol/stdin" ...
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/system" ...

        Await(ConnectReceiver(StdinProtocol))
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            OperandRef(lyric_common::SymbolPath::fromString("ReceiverConnection")))));

    ASSERT_TRUE (stdinTransport->connectCompleted());
}

TEST_F(StdStdinProtocol, EvaluateReceivePayload)
{
    stdinTransport->replyAfter(absl::Seconds(1), tempo_utils::MemoryBytes::copy("hello, world!"));
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/conn" ...
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/protocol/stdin" ...
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/system" ...

        val conn = expect Await(ConnectReceiver(StdinProtocol))
        Await(conn.ReceiveRaw())
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            OperandBytes("hello, world!"))));

    ASSERT_TRUE (stdinTransport->connectCompleted());
}
