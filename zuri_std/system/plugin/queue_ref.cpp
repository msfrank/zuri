/* SPDX-License-Identifier: BSD-3-Clause */

#include <absl/strings/substitute.h>

#include <lyric_object/bytecode_iterator.h>
#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/big_endian.h>
#include <tempo_utils/log_stream.h>

#include "future_ref.h"
#include "queue_ref.h"

QueueRef::QueueRef(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable)
{
}

QueueRef::~QueueRef()
{
    TU_LOG_INFO << "free" << QueueRef::toString();
}

lyric_runtime::DataCell
QueueRef::getField(const lyric_runtime::DataCell &field) const
{
    return {};
}

lyric_runtime::DataCell
QueueRef::setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value)
{
    return {};
}

std::string
QueueRef::toString() const
{
    return absl::Substitute("<$0: QueueRef>", this);
}

bool
QueueRef::push(const lyric_runtime::DataCell &element)
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
QueueRef::containsAvailableElement() const
{
    return m_waiting.empty() && !m_elements.empty();
}

lyric_runtime::DataCell
QueueRef::takeAvailableElement()
{
    TU_ASSERT (!m_elements.empty());
    auto next = m_elements.front();
    m_elements.erase(m_elements.begin());
    return next;
}

bool
QueueRef::waitForPush(std::shared_ptr<lyric_runtime::Promise> promise, uv_async_t *async)
{
    m_waiting.push_back({promise, async});
    return true;
}

void
QueueRef::setMembersReachable()
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
QueueRef::clearMembersReachable()
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
queue_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();
    const auto *vtable = frame.getVirtualTable();
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<QueueRef>(vtable);
    currentCoro->pushData(ref);

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
queue_push(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &arg0 = frame.getArgument(0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver != nullptr);
    auto *instance = static_cast<QueueRef *>(receiver);
    auto ret = instance->push(arg0);
    currentCoro->pushData(lyric_runtime::DataCell(ret));

    return lyric_runtime::InterpreterStatus::ok();
}

static void
on_async_complete(lyric_runtime::Promise *promise)
{
    // do nothing
}

tempo_utils::Status
queue_pop(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();
    auto *segmentManager = state->segmentManager();
    auto *scheduler = state->systemScheduler();

    auto &frame = currentCoro->peekCall();
    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver != nullptr);
    auto *instance = static_cast<QueueRef *>(receiver);

    // resolve the virtual table for Future
    auto *segment = currentCoro->peekSP();
    auto object = segment->getObject().getObject();
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
    auto promise = lyric_runtime::Promise::create(on_async_complete);

    // register an async waiter
    uv_async_t *async = nullptr;
    scheduler->registerAsync(&async, promise);

    // attach the promise to the future
    fut->prepareFuture(promise);

    // special case: if the queue has an available element then take it and set the future immediately
    if (instance->containsAvailableElement()) {
        promise->complete(instance->takeAvailableElement());
        return lyric_runtime::InterpreterStatus::ok();
    }

    // add the future to the queue
    instance->waitForPush(promise, async);

    return lyric_runtime::InterpreterStatus::ok();
}
