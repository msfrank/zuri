#ifndef ZURI_TEST_TEST_TRANSPORT_H
#define ZURI_TEST_TEST_TRANSPORT_H

#include <lyric_runtime/port_multiplexer.h>

namespace zuri_test {

    class TestTransport : public lyric_runtime::AbstractTransport {
    public:
        tempo_utils::Status connect(
            std::shared_ptr<lyric_runtime::AbstractStream> stream,
            const tempo_utils::Url &nodeUrl) override;
    };

    class SingleStreamTestTransport : public lyric_runtime::AbstractTransport, public lyric_runtime::AbstractPeer {
    public:
        tempo_utils::Status connect(
            std::shared_ptr<lyric_runtime::AbstractStream> stream,
            const tempo_utils::Url &nodeUrl) override;

        tempo_utils::Status send(std::shared_ptr<const tempo_utils::ImmutableBytes> payload) override;
        tempo_utils::Status shutdown() override;
        void reset() override;

        bool connectCompleted() const;
        tempo_utils::Url getNodeUrl() const;

        std::vector<std::shared_ptr<const tempo_utils::ImmutableBytes>> getSent();
        int numSent() const;

    private:
        bool m_connectCompleted = false;
        bool m_peerShutdown = false;
        bool m_peerReset = false;
        tempo_utils::Url m_nodeUrl;
        std::vector<std::shared_ptr<const tempo_utils::ImmutableBytes>> m_sent;
    };
}

#endif // ZURI_TEST_TEST_TRANSPORT_H