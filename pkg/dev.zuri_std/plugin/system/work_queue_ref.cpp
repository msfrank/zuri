/* SPDX-License-Identifier: BSD-3-Clause */

#include <absl/strings/substitute.h>

#include <lyric_object/bytecode_iterator.h>
#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/big_endian.h>
#include <tempo_utils/log_stream.h>

#include "future_ref.h"
#include "work_queue_ref.h"

WorkQueueRef::WorkQueueRef(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable)
{
}

WorkQueueRef::~WorkQueueRef()
{
    TU_LOG_INFO << "free" << WorkQueueRef::toString();
}

lyric_runtime::DataCell
WorkQueueRef::getField(const lyric_runtime::DataCell &field) const
{
    return {};
}

lyric_runtime::DataCell
WorkQueueRef::setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value)
{
    return {};
}

std::string
WorkQueueRef::toString() const
{
    return absl::Substitute("<$0: WorkQueue>", this);
}

bool
WorkQueueRef::push(const lyric_runtime::DataCell &element)
{
    // if there are no registered futures then push the element and return
    if (m_waiting.empty()) {
        m_elements.push_back(element);
        return true;
    }

    // otherwise pop the topmost future
    auto waiting = m_waiting.front();
    m_waiting.erase(m_waiting.begin());

    // set the result of the future and signal the scheduler to resume task
    waiting.first->complete(element);
    uv_async_send(waiting.second);

    return true;
}

bool
WorkQueueRef::containsAvailableElement() const
{
    return m_waiting.empty() && !m_elements.empty();
}

lyric_runtime::DataCell
WorkQueueRef::takeAvailableElement()
{
    TU_ASSERT (!m_elements.empty());
    auto next = m_elements.front();
    m_elements.erase(m_elements.begin());
    return next;
}

bool
WorkQueueRef::waitForPush(std::shared_ptr<lyric_runtime::Promise> promise, uv_async_t *async)
{
    m_waiting.push_back({promise, async});
    return true;
}

void
WorkQueueRef::setMembersReachable()
{
    for (auto &element : m_elements) {
        if (element.type == lyric_runtime::DataCellType::REF) {
            element.data.ref->setReachable();
        }
    }
//    for (auto &waiting : m_waiting) {
//        waiting.first->setReachable();
//    }
}

void
WorkQueueRef::clearMembersReachable()
{
    for (auto &element : m_elements) {
        if (element.type == lyric_runtime::DataCellType::REF) {
            element.data.ref->clearReachable();
        }
    }
//    for (auto &waiting : m_waiting) {
//        waiting.first->clearReachable();
//    }
}

tempo_utils::Status
work_queue_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    TU_ASSERT(vtable != nullptr);
    auto *currentCoro = state->currentCoro();
    auto ref = state->heapManager()->allocateRef<WorkQueueRef>(vtable);
    currentCoro->pushData(ref);
    return {};
}

tempo_utils::Status
work_queue_push(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &arg0 = frame.getArgument(0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<WorkQueueRef *>(receiver.data.ref);
    auto ret = instance->push(arg0);
    currentCoro->pushData(lyric_runtime::DataCell(ret));

    return {};
}

static void
on_async_accept(
    lyric_runtime::Promise *promise,
    const lyric_runtime::Waiter *waiter,
    lyric_runtime::InterpreterState *state)
{
    // do nothing
}

tempo_utils::Status
work_queue_pop(
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
    auto *instance = static_cast<WorkQueueRef *>(receiver.data.ref);

    // resolve the virtual table for Future
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

    // create a new future to wait for receive result and push it onto the top of the stack
    auto ref = state->heapManager()->allocateRef<FutureRef>(vtable);
    currentCoro->pushData(ref);
    auto *fut = static_cast<FutureRef *>(ref.data.ref);

    //
    auto promise = lyric_runtime::Promise::create(on_async_accept);

    // register an async waiter
    uv_async_t *async = nullptr;
    scheduler->registerAsync(&async, promise);

    // attach the promise to the future
    fut->prepareFuture(promise);

    // special case: if the queue has an available element then take it and set the future immediately
    if (instance->containsAvailableElement()) {
        promise->complete(instance->takeAvailableElement());
        return {};
    }

    // add the future to the queue
    instance->waitForPush(promise, async);

    return {};
}
