/* SPDX-License-Identifier: BSD-3-Clause */

#include <absl/strings/substitute.h>

#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/log_stream.h>

#include "future_ref.h"

FutureRef::FutureRef(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable),
      m_state(FutureState::Initial),
      m_waiter(nullptr),
      m_cb(nullptr),
      m_data(nullptr),
      m_result(),
      m_resolveStatus(ResolveStatus::Invalid)
{
}

FutureRef::~FutureRef()
{
    if (m_cb) {
        lyric_runtime::DataCell result;
        auto resolveResult = m_cb(result, nullptr, nullptr, nullptr, m_data);
        TU_LOG_ERROR_IF (resolveResult.isStatus()) << "resolve callback failed: " << resolveResult.getStatus();
    }
    m_cb = nullptr;
    m_data = nullptr;
    TU_LOG_INFO << "free" << FutureRef::toString();
}

lyric_runtime::DataCell
FutureRef::getField(const lyric_runtime::DataCell &field) const
{
    return lyric_runtime::DataCell();
}

lyric_runtime::DataCell
FutureRef::setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value)
{
    return lyric_runtime::DataCell();
}

bool
FutureRef::attachWaiter(lyric_runtime::Waiter *waiter)
{
    // if the future is not in Initial state then we don't attach the waiter
    if (m_state != FutureState::Initial)
        return false;

    m_waiter = waiter;
    m_state = FutureState::Ready;
    return true;
}

bool
FutureRef::releaseWaiter(lyric_runtime::Waiter **waiter)
{
    TU_ASSERT (waiter != nullptr);

    // if the future is not in Ready state then we don't release the waiter
    if (m_state != FutureState::Ready)
        return false;

    *waiter = m_waiter;
    m_waiter = nullptr;
    m_state = FutureState::Ready;
    return true;
}

bool
FutureRef::resolveFuture(
    lyric_runtime::DataCell &result,
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state)
{
    // if the future is not in Completed state then we don't resolve the future
    if (m_state != FutureState::Completed)
        return false;

    // if there is a resolve callback then run it to get the result
    if (m_cb) {
        auto resolveResult = m_cb(m_result, interp, state, this, m_data);
        m_cb = nullptr;
        m_data = nullptr;
        if (resolveResult.isStatus()) {
            TU_LOG_ERROR << "resolve callback failed: " << resolveResult.getStatus();
            m_resolveStatus = ResolveStatus::Failed;
            return false;
        }
        m_resolveStatus = resolveResult.getResult();
    }

    result = m_result;
    return true;
}

std::string
FutureRef::toString() const
{
    return absl::Substitute("<$0: FutureRef>", this);
}

FutureState
FutureRef::getState() const
{
    return m_state;
}

ResolveStatus
FutureRef::getResolveStatus() const
{
    return m_resolveStatus;
}

lyric_runtime::DataCell
FutureRef::getResult() const
{
    return m_result;
}

bool
FutureRef::complete(const lyric_runtime::DataCell &result)
{
    switch (m_state) {
        case FutureState::Initial:
        case FutureState::Ready:
        case FutureState::Waiting:
            m_state = FutureState::Completed;
            m_resolveStatus = ResolveStatus::Completed;
            m_result = result;
            return true;
        default:
            return false;
    }
}

bool
FutureRef::complete(ResolveCallback cb, void *data)
{
    switch (m_state) {
        case FutureState::Initial:
        case FutureState::Ready:
        case FutureState::Waiting:
            m_state = FutureState::Completed;
            m_resolveStatus = ResolveStatus::Completed;
            m_cb = cb;
            m_data = data;
            return true;
        default:
            return false;
    }
}

void
FutureRef::setMembersReachable()
{
    if (m_result.type == lyric_runtime::DataCellType::REF)
        m_result.data.ref->setReachable();
}

void
FutureRef::clearMembersReachable()
{
    if (m_result.type == lyric_runtime::DataCellType::REF)
        m_result.data.ref->clearReachable();
}

tempo_utils::Status
future_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();
    const auto *vtable = frame.getVirtualTable();
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<FutureRef>(vtable);
    currentCoro->pushData(ref);

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
future_ctor(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();
    TU_ASSERT (frame.numArguments() == 0);
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver != nullptr);
    //auto *instance = static_cast<FutureRef *>(receiver);

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
future_resolve(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 1);
    const auto arg0 = frame.getArgument(0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver != nullptr);
    auto *instance = static_cast<FutureRef *>(receiver);
    instance->complete(arg0);

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
future_reject(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 1);
    const auto arg0 = frame.getArgument(0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver != nullptr);
    auto *instance = static_cast<FutureRef *>(receiver);
    instance->complete(arg0);

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
future_cancel(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 1);
    const auto arg0 = frame.getArgument(0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver != nullptr);
    auto *instance = static_cast<FutureRef *>(receiver);
    instance->complete(arg0);

    return lyric_runtime::InterpreterStatus::ok();
}
