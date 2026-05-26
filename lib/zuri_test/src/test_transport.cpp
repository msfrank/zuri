
#include <zuri_test/test_transport.h>

tempo_utils::Status
zuri_test::TestTransport::connect(
    std::shared_ptr<lyric_runtime::AbstractStream> stream,
    const tempo_utils::Url &nodeUrl)
{
    return {};
}

tempo_utils::Status
zuri_test::SingleStreamTestTransport::connect(
    std::shared_ptr<lyric_runtime::AbstractStream> stream,
    const tempo_utils::Url &nodeUrl)
{
    m_nodeUrl = nodeUrl;
    stream->connectComplete(this);
    m_connectCompleted = true;
    return {};
}

tempo_utils::Status
zuri_test::SingleStreamTestTransport::send(std::shared_ptr<const tempo_utils::ImmutableBytes> payload)
{
    m_sent.push_back(payload);
    return {};
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