#ifndef ZURI_TEST_TEST_TRANSPORT_H
#define ZURI_TEST_TEST_TRANSPORT_H

#include <thread>

#include <absl/synchronization/notification.h>
#include <gmock/gmock.h>

#include <lyric_runtime/port_multiplexer.h>

namespace zuri_test {

    struct Reply {
        absl::Time at;
        std::shared_ptr<const tempo_utils::ImmutableBytes> payload;
        bool operator<(const Reply &other) const { return at < other.at; }
    };

    class AsyncReplier {
    public:
        explicit AsyncReplier(absl::Duration timeout = absl::Seconds(30));
        ~AsyncReplier();

        void start(std::shared_ptr<lyric_runtime::AbstractStream> stream);
        void stop();
        void reply(const Reply &reply);

    private:
        absl::Duration m_timeout;

        struct Priv;
        std::unique_ptr<Priv> m_priv;

        struct Priv {
            std::thread thread;
            absl::Mutex lock;
            absl::Time until;
            absl::Notification stop;
            std::shared_ptr<lyric_runtime::AbstractStream> stream ABSL_GUARDED_BY(lock);
            std::priority_queue<Reply> replies ABSL_GUARDED_BY(lock);
        };

        friend void run_replier(void *data);
    };

    class SingleStreamTestTransport : public lyric_runtime::AbstractTransport, public lyric_runtime::AbstractPeer {
    public:
        SingleStreamTestTransport() = default;

        tempo_utils::Status connect(
            std::shared_ptr<lyric_runtime::AbstractStream> stream,
            const tempo_utils::Url &nodeUrl) override;

        tempo_utils::Status send(std::shared_ptr<const tempo_utils::ImmutableBytes> payload) override;
        tempo_utils::Status shutdown() override;
        void reset() override;

        void reply(std::shared_ptr<const tempo_utils::ImmutableBytes> payload);
        void replyAfter(
            absl::Duration after,
            std::shared_ptr<const tempo_utils::ImmutableBytes> payload);

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
        AsyncReplier m_replier;
    };

}

#endif // ZURI_TEST_TEST_TRANSPORT_H