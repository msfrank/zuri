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
}

#endif // ZURI_TEST_TEST_TRANSPORT_H