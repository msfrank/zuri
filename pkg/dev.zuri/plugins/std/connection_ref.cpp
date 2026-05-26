
#include <absl/strings/substitute.h>

#include <lyric_runtime/bytes_ref.h>
#include <tempo_utils/memory_bytes.h>

#include "connection_ref.h"

ConnectionRef::ConnectionRef(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable)
{
}

ConnectionRef::~ConnectionRef()
{
    TU_LOG_V << "free ConnectionRef" << ConnectionRef::toString();
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
    return m_conn->send(payload);
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
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::Ref);
    auto *instance = static_cast<ConnectionRef *>(receiver.data.ref);

    TU_ASSERT (frame.numArguments() == 1);
    auto arg0 = frame.getArgument(0);
    TU_ASSERT (arg0.type == lyric_runtime::DataCellType::Bytes);
    auto *bytes = arg0.data.bytes;

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
    return {};
}
