
#include <tempo_utils/log_console.h>
#include <tempo_utils/log_stream.h>
#include <zuri_run/log_proto_writer.h>

zuri_run::LogProtoWriter::LogProtoWriter(bool logToStderr)
    : m_logToStderr(logToStderr)
{
}

tempo_utils::Status
zuri_run::LogProtoWriter::write(std::shared_ptr<tempo_utils::ImmutableBytes> payload)
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
