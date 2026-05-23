/* SPDX-License-Identifier: BSD-3-Clause */

#include <span>

#include <absl/strings/substitute.h>

#include <lyric_runtime/bytes_ref.h>
#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/log_stream.h>
#include <tempo_utils/memory_bytes.h>
#include <tempo_utils/unicode.h>

#include "future_ref.h"
#include "port_ref.h"

PortRef::PortRef(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable),
      m_interp(nullptr),
      m_state(nullptr)
{
}

PortRef::PortRef(
    const lyric_runtime::VirtualTable *vtable,
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state)
    : BaseRef(vtable),
      m_interp(interp),
      m_state(state)
{
    TU_ASSERT (m_interp != nullptr);
    TU_ASSERT (m_state != nullptr);
}

PortRef::PortRef(
    const lyric_runtime::VirtualTable *vtable,
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    std::shared_ptr<lyric_runtime::Connection> conn)
    : BaseRef(vtable),
      m_interp(interp),
      m_state(state),
      m_conn(std::move(conn))
{
    TU_NOTNULL (m_interp);
    TU_NOTNULL (m_state);
    TU_NOTNULL (m_conn);
}

PortRef::~PortRef()
{
    TU_LOG_INFO << "free" << PortRef::toString();
}

std::string
PortRef::toString() const
{
    return absl::Substitute("<$0: Port>", this);
}

std::shared_ptr<lyric_runtime::Connection>
PortRef::duplexPort()
{
    return m_conn;
}

bool
PortRef::send(std::shared_ptr<tempo_utils::ImmutableBytes> payload)
{
    m_conn->send(std::move(payload));
    return true;
}

// bool
// PortRef::waitForReceive(std::shared_ptr<lyric_runtime::Promise> promise, uv_async_t *async)
// {
//     if (m_port) {
//         // if port is attached then signal the port that we are ready to receive
//         m_port->readyToReceive(async);
//     } else {
//         // otherwise if this is the null protocol then signal a receive immediately
//         promise->complete(lyric_runtime::DataCell::nil());
//         uv_async_send(async);
//     }
//
//     return true;
// }

void
PortRef::setMembersReachable()
{
    for (auto &value : m_values) {
        if (value.type == lyric_runtime::DataCellType::Ref)
            value.data.ref->setReachable();
    }
    //if (m_fut)
    //    m_fut->setReachable();
}

void
PortRef::clearMembersReachable()
{
    for (auto &value : m_values) {
        if (value.type == lyric_runtime::DataCellType::Ref)
            value.data.ref->clearReachable();
    }
    //if (m_fut)
    //    m_fut->clearReachable();
}

tempo_utils::Status
std_port_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    TU_ASSERT(vtable != nullptr);
    auto *currentCoro = state->currentCoro();
    auto ref = state->heapManager()->allocateRef<PortRef>(vtable);
    currentCoro->pushData(ref);
    return {};
}

tempo_utils::Status
std_port_send(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &arg0 = frame.getArgument(0);
    if (arg0.type != lyric_runtime::DataCellType::Bytes)
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid bytes");
    auto *bytes = arg0.data.bytes;

    tu_int32 size = 0;
    TU_ASSERT (bytes->rawSize(size));
    std::vector<tu_uint8> payloadBytes(size);
    bytes->rawCopy(0, (char *) payloadBytes.data(), payloadBytes.size());
    auto payload = tempo_utils::MemoryBytes::create(std::move(payloadBytes));

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::Ref);
    auto *instance = static_cast<PortRef *>(receiver.data.ref);
    auto ret = instance->send(payload);
    currentCoro->pushData(lyric_runtime::DataCell(ret));

    return {};
}

class ReceiveCompleter : public lyric_runtime::AbstractReceiveCompleter {
public:
    ReceiveCompleter(
        std::shared_ptr<lyric_runtime::Promise> promise,
        lyric_runtime::HeapManager *heapManager)
        : m_promise(std::move(promise)),
          m_heapManager(heapManager)
    {
        TU_NOTNULL (m_promise);
        TU_NOTNULL (m_heapManager);
    }
    void
    receiveComplete(std::shared_ptr<const tempo_utils::ImmutableBytes> payload) override
    {
        auto result = m_heapManager->allocateBytes(payload->getSpan());
        m_promise->complete(result);
    }
    void
    error(const tempo_utils::Status &status) override
    {
        auto result = m_heapManager->allocateStatus(status.getStatusCode(), status.getMessage());
        m_promise->reject(result);
    }
    void close() override
    {
    }
private:
    std::shared_ptr<lyric_runtime::Promise> m_promise;
    lyric_runtime::HeapManager *m_heapManager;
};

// static void
// on_async_accept(
//     lyric_runtime::Promise *promise,
//     const lyric_runtime::Waiter *waiter,
//     lyric_runtime::InterpreterState *state)
// {
//     auto *completer = static_cast<ReceiveCompleter *>(promise->getData());
//     if (completer->isComplete()) {
//         promise->complete(completer->getResult());
//     } else {
//         promise->reject(completer->getResult());
//     }
// }

tempo_utils::Status
std_port_receive(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *)
{
    auto *currentCoro = state->currentCoro();
    auto *segmentManager = state->segmentManager();
    auto *systemScheduler = state->systemScheduler();
    auto *heapManager = state->heapManager();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::Ref);
    auto *instance = static_cast<PortRef *>(receiver.data.ref);

    //
    auto *segment = currentCoro->peekSP();
    auto object = segment->getObject();
    auto symbol = object.findSymbol(lyric_common::SymbolPath({"Future"}));
    TU_ASSERT (symbol.isValid());
    lyric_runtime::InterpreterStatus status;
    auto descriptor = segmentManager->resolveDescriptor(segment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    TU_ASSERT (descriptor.type == lyric_runtime::DataCellType::Descriptor);
    TU_ASSERT (descriptor.data.descriptor->getLinkageSection() == lyric_object::LinkageSection::Class);
    const auto *vtable = segmentManager->resolveClassVirtualTable(descriptor, status);
    TU_ASSERT(vtable != nullptr);

    // create a new future to wait for receive result
    auto ref = state->heapManager()->allocateRef<FutureRef>(vtable);
    currentCoro->pushData(ref);
    auto *fut = ref.data.ref;

    auto conn = instance->duplexPort();

    // register receive
    auto promise = lyric_runtime::Promise::create();
    auto completer = std::make_shared<ReceiveCompleter>(promise, heapManager);
    conn->registerReceive(systemScheduler, promise, completer);

    // attach the waiter to the future
    fut->prepareFuture(promise);

    return {};
}
