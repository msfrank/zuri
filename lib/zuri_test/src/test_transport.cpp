
#include <zuri_test/test_transport.h>

tempo_utils::Status
zuri_test::SingleStreamTestTransport::connect(
    std::shared_ptr<lyric_runtime::AbstractStream> stream,
    const tempo_utils::Url &nodeUrl)
{
    m_nodeUrl = nodeUrl;
    stream->connectComplete(this);
    m_replier.start(stream);
    m_connectCompleted = true;
    return {};
}

tempo_utils::Status
zuri_test::SingleStreamTestTransport::send(std::shared_ptr<const tempo_utils::ImmutableBytes> payload)
{
    m_sent.push_back(payload);
    return {};
}

void
zuri_test::SingleStreamTestTransport::reply(std::shared_ptr<const tempo_utils::ImmutableBytes> payload)
{
    Reply reply{ absl::Now(), payload};
    m_replier.reply(reply);
}

void
zuri_test::SingleStreamTestTransport::replyAfter(
    absl::Duration after,
    std::shared_ptr<const tempo_utils::ImmutableBytes> payload)
{
    Reply reply{ absl::Now() + after, payload};
    m_replier.reply(reply);
}

tempo_utils::Status
zuri_test::SingleStreamTestTransport::shutdown()
{
    m_peerShutdown =  true;
    return {};
}

void
zuri_test::SingleStreamTestTransport::reset()
{
    m_peerReset = true;
}

bool
zuri_test::SingleStreamTestTransport::connectCompleted() const
{
    return m_connectCompleted;
}

tempo_utils::Url
zuri_test::SingleStreamTestTransport::getNodeUrl() const
{
    return m_nodeUrl;
}

std::vector<std::shared_ptr<const tempo_utils::ImmutableBytes>>
zuri_test::SingleStreamTestTransport::getSent()
{
    return m_sent;
}

int
zuri_test::SingleStreamTestTransport::numSent() const
{
    return m_sent.size();
}

zuri_test::AsyncReplier::AsyncReplier(absl::Duration timeout)
    : m_timeout(timeout),
      m_priv(std::make_unique<Priv>())
{
}

zuri_test::AsyncReplier::~AsyncReplier()
{
    stop();
}

constexpr tu_int64 kSleepMillis = 100;

void
zuri_test::run_replier(void *data)
{
    auto *priv = static_cast<AsyncReplier::Priv *>(data);

    while (!priv->stop.WaitForNotificationWithTimeout(absl::Milliseconds(kSleepMillis))) {

        auto now = absl::Now();
        if (priv->until < now)
            break;

        absl::MutexLock locker(&priv->lock);

        now = absl::Now();
        while (!priv->replies.empty()) {
            const auto &reply = priv->replies.top();
            if (now < reply.at)
                break;
            priv->stream->receiveComplete(reply.payload);
            priv->replies.pop();
        }
    }
}

void
zuri_test::AsyncReplier::start(std::shared_ptr<lyric_runtime::AbstractStream> stream)
{
    TU_NOTNULL (m_priv);
    m_priv->until = absl::Now() + m_timeout;
    m_priv->stream = std::move(stream);

    // this starts  the thread
    m_priv->thread = std::thread(run_replier, m_priv.get());
}

void
zuri_test::AsyncReplier::stop()
{
    if (!m_priv)
        return;
    m_priv->stop.Notify();
    m_priv->thread.join();
    m_priv.reset();
}

void
zuri_test::AsyncReplier::reply(const Reply &reply)
{
    TU_NOTNULL (m_priv);
    absl::MutexLock locker(&m_priv->lock);
    m_priv->replies.push(reply);
}
