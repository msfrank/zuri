/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_runtime/protocol_ref.h>
#include <tempo_utils/log_stream.h>
#include <tempo_utils/uuid.h>

#include "conn_traps.h"
#include "connection_ref.h"
#include "future_ref.h"
#include "url_ref.h"

class ConnectCompleter : public lyric_runtime::AbstractConnectCompleter {
public:
    void connectComplete() override
    {
        TU_LOG_INFO << "connect complete";
    }
    void error(const tempo_utils::Status &status) override
    {
        TU_LOG_INFO << "connect error: " << status;
        m_status = status;
    }
    void close() override { }
    tempo_utils::Status getStatus() { return m_status; }
private:
    tempo_utils::Status m_status;
};

class ConnectOps : public lyric_runtime::PromiseOperations {
public:
    ConnectOps(const lyric_runtime::Operand &ref, std::shared_ptr<ConnectCompleter> completer)
        : m_ref(ref),
          m_completer(std::move(completer))
    {}
    void onAccept(
        lyric_runtime::Promise *promise,
        const lyric_runtime::Waiter *waiter,
        lyric_runtime::InterpreterState *state) override
    {
        auto status = m_completer->getStatus();
        if (status.notOk()) {
            auto *heapManager = state->heapManager();
            auto rejected = heapManager->allocateStatus(status.getStatusCode(), status.getMessage());
            promise->reject(rejected);
        } else {
            promise->complete(m_ref);
        }
    }
    void setReachable() override
    {
        m_ref.setReachable();
    }
private:
    lyric_runtime::Operand m_ref;
    std::shared_ptr<ConnectCompleter> m_completer;
};

tempo_utils::Status
std_conn_make_connection(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *)
{
    auto *scheduler = state->systemScheduler();
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 2);

    const auto &arg0 = frame.getArgument(0);
    lyric_runtime::ProtocolRef *protocol;
    TU_ASSERT(arg0.getProtocol(protocol));

    const auto &arg1 = frame.getArgument(1);
    UrlRef *url;
    TU_ASSERT(arg1.castRef(url));

    lyric_runtime::Operand sender;
    TU_RETURN_IF_NOT_OK (currentCoro->popData(sender));
    ConnectionRef *instance;
    TU_ASSERT (sender.castRef(instance));

    lyric_runtime::Operand top;
    TU_RETURN_IF_NOT_OK (currentCoro->peekData(top));
    FutureRef *fut;
    TU_ASSERT (top.castRef(fut));

    // make the connection
    auto *portMultiplexer = state->portMultiplexer();
    std::shared_ptr<lyric_runtime::Connection> connection;
    TU_ASSIGN_OR_RETURN (connection, portMultiplexer->makeConnection(protocol->getSymbolUrl().toUrl(), url->getUrl()));
    instance->setConnection(connection);

    // register connect
    auto completer = std::make_shared<ConnectCompleter>();
    auto ops = std::make_unique<ConnectOps>(sender, completer);
    auto promise = lyric_runtime::Promise::create(std::move(ops));
    connection->registerConnect(scheduler, promise, completer);

    // attach the connect promise to the future
    fut->prepareFuture(promise);

    return {};
}
