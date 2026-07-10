
#include <absl/strings/substitute.h>

#include <lyric_runtime/bytes_ref.h>
#include <tempo_utils/memory_bytes.h>

#include "connection_ref.h"

#include "future_ref.h"

ConnectionRef::ConnectionRef(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable)
{
}

ConnectionRef::~ConnectionRef()
{
    TU_LOG_V << "free ConnectionRef" << ConnectionRef::toString();
}

tu_uint64
ConnectionRef::getTypeTag() const
{
    return type_tag();
}

std::string
ConnectionRef::toString() const
{
    if (m_conn) {
        return absl::Substitute("<$0: Connection $1>", this, m_conn->getId().toRfc4122String());
    }
    return absl::Substitute("<$0: Connection>", this);
}

std::shared_ptr<lyric_runtime::Connection>
ConnectionRef::getConnection() const
{
    return m_conn;
}

void
ConnectionRef::setConnection(std::shared_ptr<lyric_runtime::Connection> connection)
{
    m_conn = std::move(connection);
}

tempo_utils::Status
ConnectionRef::send(std::shared_ptr<const tempo_utils::ImmutableBytes> payload)
{
    if (m_conn == nullptr)
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant,
            "invalid connection state");
    return m_conn->send(std::move(payload));
}

class ReceiveCompleter : public lyric_runtime::AbstractReceiveCompleter {
public:
    ReceiveCompleter(ConnectionRef *conn)
        : m_conn(conn)
    {
        TU_NOTNULL (m_conn);
    }
    void receiveComplete(std::shared_ptr<const tempo_utils::ImmutableBytes> payload) override
    {
        m_payload = std::move(payload);
    }
    void error(const tempo_utils::Status &status) override
    {
        m_status = status;
    }
    void close() override {
    }
    std::shared_ptr<const tempo_utils::ImmutableBytes> getPayload() { return m_payload; }
    tempo_utils::Status getStatus() { return m_status; }
private:
    ConnectionRef *m_conn = nullptr;
    std::shared_ptr<const tempo_utils::ImmutableBytes> m_payload;
    tempo_utils::Status m_status;
};

class ReceiveOps : public lyric_runtime::PromiseOperations {
public:
    ReceiveOps(ConnectionRef *conn, std::shared_ptr<ReceiveCompleter> completer)
        : m_conn(conn),
          m_completer(std::move(completer))
    {
        TU_NOTNULL (m_conn);
        TU_NOTNULL (m_completer);
    }
    void onAccept(
        lyric_runtime::Promise *promise,
        const lyric_runtime::Waiter *waiter,
        lyric_runtime::InterpreterState *state) override
    {
        auto *heapManager = state->heapManager();
        auto status = m_completer->getStatus();
        if (status.notOk()) {
            auto rejected = heapManager->allocateStatus(status.getStatusCode(), status.getMessage());
            promise->reject(rejected);
        } else {
            auto payload = m_completer->getPayload();
            auto completed = heapManager->allocateBytes(payload->getSpan());
            promise->complete(completed);
        }
    }
    void setReachable() override
    {
        m_conn->setReachable();
    }
private:
    ConnectionRef *m_conn;
    std::shared_ptr<ReceiveCompleter> m_completer;
};

tempo_utils::Status
ConnectionRef::receiveAsync(AbstractRef *fut, lyric_runtime::SystemScheduler *systemScheduler)
{
    auto completer = std::make_shared<ReceiveCompleter>(this);
    auto ops = std::make_unique<ReceiveOps>(this, completer);
    auto promise = lyric_runtime::Promise::create(std::move(ops));

    TU_RETURN_IF_NOT_OK (m_conn->registerReceive(systemScheduler, promise, completer));
    fut->prepareFuture(promise);

    return {};
}

tempo_utils::Status
ConnectionRef::shutdown()
{
    if (m_conn == nullptr)
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant,
            "invalid connection state");
    return m_conn->shutdown();
}

tempo_utils::Status
std_connection_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    TU_ASSERT(vtable != nullptr);
    auto *currentCoro = state->currentCoro();
    auto ref = state->heapManager()->allocateRef<ConnectionRef>(vtable);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(ref));
    return {};
}

tempo_utils::Status
std_connection_send(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();
    auto *heapManager = state->heapManager();

    auto &frame = currentCoro->currentCallOrThrow();

    auto receiver = frame.getReceiver();
    ConnectionRef *instance;
    TU_ASSERT(receiver.castRef(instance));

    TU_ASSERT (frame.numArguments() == 1);
    auto arg0 = frame.getArgument(0);
    lyric_runtime::BytesRef *bytes;
    TU_ASSERT (arg0.getBytes(bytes));

    auto payload = tempo_utils::MemoryBytes::create(bytes->getBytes());
    auto status = instance->send(payload);
    TU_RETURN_IF_NOT_OK (heapManager->loadStatusOntoStack(status.getStatusCode(), status.getMessage()));

    return {};
}

tempo_utils::Status
std_connection_receive(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();
    auto *systemScheduler = state->systemScheduler();
    auto *heapManager = state->heapManager();

    auto &frame = currentCoro->currentCallOrThrow();

    auto receiver = frame.getReceiver();
    ConnectionRef *instance;
    TU_ASSERT(receiver.castRef(instance));

    TU_ASSERT (frame.numArguments() == 0);

    lyric_runtime::Operand data;
    TU_RETURN_IF_NOT_OK (currentCoro->peekData(data));
    FutureRef *fut;
    TU_ASSERT (data.castRef(fut));

    auto status = instance->receiveAsync(fut, systemScheduler);
    TU_RETURN_IF_NOT_OK (heapManager->loadStatusOntoStack(status.getStatusCode(), status.getMessage()));

    return {};
}
