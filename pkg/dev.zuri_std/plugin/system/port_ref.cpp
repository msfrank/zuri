/* SPDX-License-Identifier: BSD-3-Clause */

#include <absl/strings/substitute.h>

#include <lyric_runtime/bytes_ref.h>
#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/big_endian.h>
#include <tempo_utils/log_stream.h>
#include <tempo_utils/unicode.h>

#include "future_ref.h"
#include "port_ref.h"

#include <tempo_utils/memory_bytes.h>

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
    std::shared_ptr<lyric_runtime::DuplexPort> port)
    : BaseRef(vtable),
      m_interp(interp),
      m_state(state),
      m_port(port)
{
    TU_ASSERT (m_interp != nullptr);
    TU_ASSERT (m_state != nullptr);
    TU_ASSERT (port != nullptr);
}

PortRef::~PortRef()
{
    TU_LOG_INFO << "free" << PortRef::toString();
}

lyric_runtime::DataCell
PortRef::getField(const lyric_runtime::DataCell &field) const
{
    return {};
}

lyric_runtime::DataCell
PortRef::setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value)
{
    return {};
}

std::string
PortRef::toString() const
{
    return absl::Substitute("<$0: Port>", this);
}

std::shared_ptr<lyric_runtime::DuplexPort>
PortRef::duplexPort()
{
    return m_port;
}

bool
PortRef::send(std::shared_ptr<tempo_utils::ImmutableBytes> payload)
{
    // if port is not attached then this is the null protocol, so drop the message
    if (!m_port)
        return true;
    // otherwise if a port is attached, then send the message through it
    m_port->send(std::move(payload));
    return true;
}

bool
PortRef::waitForReceive(std::shared_ptr<lyric_runtime::Promise> promise, uv_async_t *async)
{
    if (m_port) {
        // if port is attached then signal the port that we are ready to receive
        m_port->readyToReceive(async);
    } else {
        // otherwise if this is the null protocol then signal a receive immediately
        promise->complete(lyric_runtime::DataCell::nil());
        uv_async_send(async);
    }

    return true;
}

void
PortRef::setMembersReachable()
{
    for (auto &value : m_values) {
        if (value.type == lyric_runtime::DataCellType::REF)
            value.data.ref->setReachable();
    }
    //if (m_fut)
    //    m_fut->setReachable();
}

void
PortRef::clearMembersReachable()
{
    for (auto &value : m_values) {
        if (value.type == lyric_runtime::DataCellType::REF)
            value.data.ref->clearReachable();
    }
    //if (m_fut)
    //    m_fut->clearReachable();
}

tempo_utils::Status
port_alloc(
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
port_send(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &arg0 = frame.getArgument(0);
    if (arg0.type != lyric_runtime::DataCellType::BYTES)
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid bytes");
    auto *bytes = arg0.data.bytes;
    auto payload = tempo_utils::MemoryBytes::copy(
        std::span(bytes->getBytesData(), bytes->getBytesSize()));

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<PortRef *>(receiver.data.ref);
    auto ret = instance->send(payload);
    currentCoro->pushData(lyric_runtime::DataCell(ret));

    return {};
}

struct ResolveData {
    std::shared_ptr<lyric_runtime::DuplexPort> port;
    std::shared_ptr<tempo_utils::ImmutableBytes> payload;
    lyric_runtime::DataCell ref;
};

static void
complete_promise(
    lyric_runtime::Promise *promise,
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state)
{
    auto *heapManager = state->heapManager();

    auto *data = static_cast<ResolveData *>(promise->getData());

    // construct a Bytes instance containing the payload
    auto payload = heapManager->allocateBytes(data->payload->getSpan());

    // free the ResolveData
    delete data;

    // complete the promise
    promise->complete(payload);
}

static void
on_async_accept(
    lyric_runtime::Promise *promise,
    const lyric_runtime::Waiter *waiter,
    lyric_runtime::InterpreterState *state)
{
    auto *data = static_cast<ResolveData *>(promise->getData());
    TU_ASSERT (data->port->hasPending());
    data->payload = data->port->nextPending();
}

tempo_utils::Status
port_receive(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *unused)
{
    auto *currentCoro = state->currentCoro();
    auto *segmentManager = state->segmentManager();
    auto *scheduler = state->systemScheduler();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<PortRef *>(receiver.data.ref);

    //
    auto *segment = currentCoro->peekSP();
    auto object = segment->getObject();
    auto symbol = object.findSymbol(lyric_common::SymbolPath({"Future"}));
    TU_ASSERT (symbol.isValid());
    lyric_runtime::InterpreterStatus status;
    auto descriptor = segmentManager->resolveDescriptor(segment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    TU_ASSERT (descriptor.type == lyric_runtime::DataCellType::CLASS);
    const auto *vtable = segmentManager->resolveClassVirtualTable(descriptor, status);
    TU_ASSERT(vtable != nullptr);

    // create a new future to wait for receive result
    auto ref = state->heapManager()->allocateRef<FutureRef>(vtable);
    currentCoro->pushData(ref);
    auto *fut = ref.data.ref;

    //
    auto *data = static_cast<ResolveData *>(std::malloc(sizeof(ResolveData)));
    data->port = instance->duplexPort();

    //
    lyric_runtime::PromiseOptions options;
    options.adapt = complete_promise;
    options.data = data;
    options.release = std::free;
    auto promise = lyric_runtime::Promise::create(on_async_accept, options);

    // register a waiter bound to the current task
    uv_async_t *async = nullptr;
    scheduler->registerAsync(&async, promise);

    // attach the waiter to the future
    fut->prepareFuture(promise);

    // set port to await a message from the remote side
    instance->waitForReceive(promise, async);

    return {};
}
