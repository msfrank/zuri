#ifndef ZURI_RUN_LOG_PROTO_WRITER_H
#define ZURI_RUN_LOG_PROTO_WRITER_H

#include <lyric_runtime/abstract_port_writer.h>

namespace zuri_run {

    constexpr const char *kLogProtocolUrl = "dev.zuri.proto:log";

    class LogProtoWriter : public lyric_runtime::AbstractPortWriter {
    public:
        explicit LogProtoWriter(bool logToStderr);

        tempo_utils::Status write(std::shared_ptr<tempo_utils::ImmutableBytes> payload) override;

    private:
        bool m_logToStderr;
    };
}

#endif // ZURI_RUN_LOG_PROTO_WRITER_H