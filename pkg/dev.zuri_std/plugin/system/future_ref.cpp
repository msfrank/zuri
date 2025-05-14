/* SPDX-License-Identifier: BSD-3-Clause */

#include <absl/strings/substitute.h>

#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/log_stream.h>

#include "future_ref.h"

FutureRef::FutureRef(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable),
      m_state(FutureState::Initial),
      m_promise(nullptr)
{
}

FutureRef::~FutureRef()
{
    TU_LOG_INFO << "free" << FutureRef::toString();
}

lyric_runtime::DataCell
FutureRef::getField(const lyric_runtime::DataCell &field) const
{
    return {};
}

lyric_runtime::DataCell
FutureRef::setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value)
{
    return {};
}

bool
FutureRef::prepareFuture(std::shared_ptr<lyric_runtime::Promise> promise)
{
    // if the future is not in Initial state then prepare fails
    if (m_state != FutureState::Initial)
        return false;

    // if future already has a promise assigned then prepare fails
    if (m_promise != nullptr)
        return false;

    // if promise is already resolved then prepare fails
    if (promise->getPromiseState() != lyric_runtime::PromiseState::Pending)
        return false;

    m_promise = promise;
    m_state = FutureState::Ready;
    return true;
}

void
FutureRef::checkState()
{
    // if the promise has been completed or rejected then mark the future as resolved and return
    switch (m_promise->getPromiseState()) {
        case lyric_runtime::PromiseState::Completed:
        case lyric_runtime::PromiseState::Rejected:
            m_state = FutureState::Resolved;
            return;
        default:
            break;
    }
}

bool
FutureRef::awaitFuture(lyric_runtime::SystemScheduler *systemScheduler)
{
    TU_ASSERT (systemScheduler != nullptr);

    // synchronize the internal state
    checkState();

    // if the future has not been prepared then await fails
    if (m_state == FutureState::Initial)
        return false;

    // if the future is already resolved out then await succeeds without suspending
    if (m_state == FutureState::Resolved)
        return true;

    if (m_state == FutureState::Ready) {
        // suspend the current task and set state to Waiting
        m_promise->await(systemScheduler);
        return true;
    }

    if (m_state == FutureState::Waiting) {
        // FIXME: support suspending multiple tasks for the same future
        TU_UNREACHABLE();
    }

    return false;
}

bool
FutureRef::resolveFuture(lyric_runtime::DataCell &result)
{
    checkState();
    if (m_state == FutureState::Resolved) {
        result = m_promise->getResult();
        return true;
    }
    return false;
}

std::string
FutureRef::toString() const
{
    return absl::Substitute("<$0: FutureRef>", this);
}

bool
FutureRef::isFinished() const
{
    return m_state == FutureState::Resolved;
}

std::shared_ptr<lyric_runtime::Promise>
FutureRef::getPromise() const
{
    return m_promise;
}

tempo_utils::Status
FutureRef::forward(uv_async_t *async)
{
    TU_ASSERT (async != nullptr);

    switch (m_state) {

        case FutureState::Initial:
        case FutureState::Ready:
        case FutureState::Waiting:
            m_deps.push_back(async);
            return {};

        default:
            return lyric_runtime::InterpreterStatus::forCondition(
                lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid future state");
    }
}

tempo_utils::Status
FutureRef::complete(const lyric_runtime::DataCell &result)
{
    switch (m_state) {

        case FutureState::Initial:
            m_promise = lyric_runtime::Promise::completed(result);
            m_state = FutureState::Resolved;
            return lyric_runtime::InterpreterStatus::ok();

        case FutureState::Ready:
        case FutureState::Waiting:
            m_promise->complete(result);
            m_state = FutureState::Resolved;
            return lyric_runtime::InterpreterStatus::ok();

        default:
            return lyric_runtime::InterpreterStatus::forCondition(
                lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid future state");
    }
}

tempo_utils::Status
FutureRef::reject(const lyric_runtime::DataCell &result)
{
    switch (m_state) {

        case FutureState::Initial:
            m_promise = lyric_runtime::Promise::rejected(result);
            m_state = FutureState::Resolved;
            return lyric_runtime::InterpreterStatus::ok();

        case FutureState::Ready:
        case FutureState::Waiting:
            m_promise->reject(result);
            m_state = FutureState::Resolved;
            return lyric_runtime::InterpreterStatus::ok();

        default:
            return lyric_runtime::InterpreterStatus::forCondition(
                lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid future state");
    }
}

void
FutureRef::setMembersReachable()
{
    auto result = m_promise->getResult();
    if (result.type == lyric_runtime::DataCellType::REF) {
        result.data.ref->setReachable();
    }
}

void
FutureRef::clearMembersReachable()
{
    auto result = m_promise->getResult();
    if (result.type == lyric_runtime::DataCellType::REF) {
        result.data.ref->clearReachable();
    }
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
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    //auto *instance = static_cast<FutureRef *>(receiver.data.ref);

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
future_complete(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 1);
    const auto arg0 = frame.getArgument(0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<FutureRef *>(receiver.data.ref);
    TU_RETURN_IF_NOT_OK (instance->complete(arg0));

    currentCoro->pushData(lyric_runtime::DataCell(true));
    return {};
}

tempo_utils::Status
future_reject(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 1);
    const auto arg0 = frame.getArgument(0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<FutureRef *>(receiver.data.ref);
    TU_RETURN_IF_NOT_OK (instance->reject(arg0));

    currentCoro->pushData(lyric_runtime::DataCell(true));
    return {};
}

tempo_utils::Status
future_cancel(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 1);
    const auto arg0 = frame.getArgument(0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<FutureRef *>(receiver.data.ref);
    TU_RETURN_IF_NOT_OK (instance->reject(arg0));

    currentCoro->pushData(lyric_runtime::DataCell(true));
    return {};
}

struct ThenData {
    lyric_runtime::DataCell result;
    lyric_runtime::DataCell fn;
};

static void
on_map_result(
    lyric_runtime::Promise *promise,
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state)
{
    auto *data = static_cast<ThenData *>(promise->getData());

    // push a new frame onto the current task call stack
    auto *currentTask = state->systemScheduler()->currentTask();
    std::vector<lyric_runtime::DataCell> args{data->result};
    if (!data->fn.data.ref->applyClosure(currentTask, args, state)) {
        TU_LOG_FATAL << "failed to apply closure";
    }

    // run the interpreter until the closure completes
    auto runClosureResult = interp->runSubinterpreter();
    if (runClosureResult.isStatus()) {
        TU_LOG_FATAL << "failed to run closure: " << runClosureResult.getStatus();
    }

    data->result = runClosureResult.getResult();

    // complete or reject the promise based on the fn result
    if (data->result.type == lyric_runtime::DataCellType::REF
        && data->result.data.ref->errorStatusCode() != tempo_utils::StatusCode::kOk) {
        promise->reject(data->result);
    } else {
        promise->complete(data->result);
    }
}

static void
on_future_complete(lyric_runtime::Promise *promise)
{
    // this callback must exist but does nothing
}

tempo_utils::Status
future_then(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();
    auto *scheduler = state->systemScheduler();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 1);
    const auto arg0 = frame.getArgument(0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<FutureRef *>(receiver.data.ref);

    // allocate the promise data
    auto *data = static_cast<ThenData *>(std::malloc(sizeof(ThenData)));
    data->result = {};
    data->fn = arg0;

    // allocate the promise
    lyric_runtime::PromiseOptions options;
    options.adapt = on_map_result;
    options.release = std::free;
    options.data = data;
    auto promise = lyric_runtime::Promise::create(on_future_complete, options);

    // register an async handle and bind it to the promise
    uv_async_t *async = nullptr;
    scheduler->registerAsync(&async, promise);

    // construct the dependent future
    const auto *vtable = instance->getVirtualTable();
    TU_ASSERT(vtable != nullptr);
    auto ref = state->heapManager()->allocateRef<FutureRef>(vtable);
    auto *fut = ref.data.ref;

    // attach the promise to the future
    fut->prepareFuture(promise);

    // if instance is already finished then immediately finish the dependent future using the result
    // from the instance promise. otherwise pass the async handle to the instance.
    if (instance->isFinished()) {
        data->result = instance->getPromise()->getResult();
        uv_async_send(async);
    } else {
        TU_RETURN_IF_NOT_OK (instance->forward(async));
    }

    // return reference to the dependent future
    currentCoro->pushData(ref);

    return {};
}
