#ifndef ZURI_RUN_LOG_TRANSPORT_H
#define ZURI_RUN_LOG_TRANSPORT_H

#include <lyric_runtime/abstract_transport.h>
#include <tempo_utils/url.h>

namespace zuri_run {

    class LogTransport : public lyric_runtime::AbstractTransport, lyric_runtime::AbstractPeer {
    public:
        explicit LogTransport(bool logToStderr);

        tempo_utils::Status connect(
            std::shared_ptr<lyric_runtime::AbstractStream> stream,
            const tempo_utils::Url &nodeUrl) override;

        tempo_utils::Status send(std::shared_ptr<const tempo_utils::ImmutableBytes> payload) override;
        tempo_utils::Status shutdown() override;
        void reset() override;

    private:
        bool m_logToStderr;
    };
}

#endif // ZURI_RUN_LOG_TRANSPORT_H