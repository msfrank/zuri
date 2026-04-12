
#include <tempo_utils/log_console.h>
#include <tempo_utils/log_stream.h>
#include <zuri_run/log_proto_writer.h>

zuri_run::LogTransport::LogTransport(bool logToStderr)
    : m_logToStderr(logToStderr)
{
}

tempo_utils::Status
zuri_run::LogTransport::connect(
    std::shared_ptr<lyric_runtime::AbstractStream> stream,
    const tempo_utils::Url &nodeUrl)
{
    stream->connectComplete(this);
    return {};
}

tempo_utils::Status
zuri_run::LogTransport::send(std::shared_ptr<const tempo_utils::ImmutableBytes> payload)
{
    if (payload != nullptr) {
        std::string utf8((const char *) payload->getData(), payload->getSize());
        if (m_logToStderr) {
            TU_CONSOLE_ERR << utf8;
        } else {
            TU_CONSOLE_OUT << utf8;
        }
    }
    return {};
}

tempo_utils::Status
zuri_run::LogTransport::shutdown()
{
    return {};
}

void
zuri_run::LogTransport::reset()
{
}