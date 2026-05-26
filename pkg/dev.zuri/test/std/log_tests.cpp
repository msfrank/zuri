#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>
#include <zuri_test_runtime/zuri_test_runtime.h>
#include <zuri_test/test_transport.h>
#include <zuri_test/zuri_tester.h>

class StdLogLog : public ::testing::Test {
protected:
    std::unique_ptr<zuri_test::ZuriTester> tester;
    std::shared_ptr<zuri_test::SingleStreamTestTransport> logTransport;

    void SetUp() override {
        auto runtime = zuri_test_runtime::get_global_test_runtime();
        zuri_test::TesterOptions options;
        options.localPackages.emplace_back(ZURI_STD_PACKAGE_PATH);

        logTransport = std::make_shared<zuri_test::SingleStreamTestTransport>();
        auto logProtocolUrl = tempo_utils::Url::fromString(ZURI_STD_PACKAGE_URL "/protocol/log#LogProtocol");
        options.localTransports[logProtocolUrl] = logTransport;
        options.protocolConnectors[logProtocolUrl] = lyric_runtime::ConnectorPolicy{};

        tester = std::make_unique<zuri_test::ZuriTester>(runtime, options);
        TU_RAISE_IF_NOT_OK (tester->configure());
    }
};

TEST_F(StdLogLog, EvaluateConnect)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/conn" ...
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/protocol/log" ...
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/system" ...

        Await(ConnectSender(LogProtocol))
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellRef(lyric_common::SymbolPath::fromString("SenderConnection")))));

    ASSERT_TRUE (logTransport->connectCompleted());
}

TEST_F(StdLogLog, EvaluateSendPayload)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/conn" ...
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/protocol/log" ...
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/system" ...

        val conn = expect Await(ConnectSender(LogProtocol))
        conn.SendRaw("hello, world!".ToBytes())
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            MatchesStatusRefCode(tempo_utils::StatusCode::kOk))));

    ASSERT_TRUE (logTransport->connectCompleted());
    auto sent = logTransport->getSent();
    ASSERT_THAT (sent, ::testing::SizeIs(1));
    auto payload = sent.at(0);
    ASSERT_EQ ("hello, world!", payload->toString());
}
