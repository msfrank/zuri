/* SPDX-License-Identifier: BSD-3-Clause */

#include <iostream>

#include <lyric_runtime/data_cell.h>
#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/url_ref.h>

#include "future_ref.h"
#include "port_ref.h"
#include "system_traps.h"

tempo_utils::Status
std_system_acquire(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &cell = frame.getArgument(0);
    TU_ASSERT(cell.type == lyric_runtime::DataCellType::URL);

    tempo_utils::Url protocolUrl;
    if (!cell.data.url->uriValue(protocolUrl))
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid protocol uri");
    if (!protocolUrl.isValid())
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid protocol uri");
    if (protocolUrl.getScheme() != "dev.zuri.proto")
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid protocol url scheme");

    //
    auto *multiplexer = state->portMultiplexer();
    auto *segmentManager = state->segmentManager();
    auto *heapManager = state->heapManager();

    auto *segment = currentCoro->peekSP();
    auto object = segment->getObject().getObject();
    auto symbol = object.findSymbol(lyric_common::SymbolPath({"Port"}));
    TU_ASSERT (symbol.isValid());
    lyric_runtime::InterpreterStatus status;
    auto descriptor = segmentManager->resolveDescriptor(segment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    TU_ASSERT (descriptor.type == lyric_runtime::DataCellType::CLASS);
    const auto *vtable = segmentManager->resolveClassVirtualTable(descriptor, status);
    TU_ASSERT(vtable != nullptr);

    // create a new port and return it to caller
    lyric_runtime::DataCell ref;
    if (protocolUrl.toString() == "dev.zuri.proto:null") {
        ref = heapManager->allocateRef<PortRef>(vtable, interp, state);
    } else {
        auto port = multiplexer->getPort(protocolUrl);
        if (port == nullptr)
            return lyric_runtime::InterpreterStatus::forCondition(
                lyric_runtime::InterpreterCondition::kRuntimeInvariant, "missing port for protocol uri");
        ref = heapManager->allocateRef<PortRef>(vtable, interp, state, port);
    }

    // push ref onto the stack and insert into the heap
    currentCoro->pushData(ref);

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
std_system_await(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();
    auto *scheduler = state->systemScheduler();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() >= 1);
    const auto &cell = frame.getArgument(0);
    TU_ASSERT(cell.type == lyric_runtime::DataCellType::REF);

    auto *fut = cell.data.ref;
    fut->awaitFuture(scheduler);

//    // consume the waiter if it exists
//    lyric_runtime::Waiter *waiter = nullptr;
//
//    if (ref->releaseWaiter(&waiter)) {
//        if (waiter->handle) {
//            // if the future has not been awaited and has not completed, then suspend the current task
//            auto *scheduler = state->systemScheduler();
//            waiter->task = scheduler->currentTask();
//            TU_LOG_INFO << "suspending task " << waiter->task;
//            scheduler->suspendTask(waiter->task);
//        }
//    }
//
//    // push the future onto the top of the stack
//    currentCoro->pushData(cell);

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
std_system_get_result(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() >= 1);
    const auto &cell = frame.getArgument(0);
    TU_ASSERT(cell.type == lyric_runtime::DataCellType::REF);
    auto *ref = cell.data.ref;

    lyric_runtime::DataCell result;
    ref->resolveFuture(result);
    currentCoro->pushData(result);

    return lyric_runtime::InterpreterStatus::ok();
}

static void
on_sleep_accept(lyric_runtime::Promise *promise)
{
    promise->complete(lyric_runtime::DataCell::nil());
}

tempo_utils::Status
std_system_sleep(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *scheduler = state->systemScheduler();
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &cell = frame.getArgument(0);
    TU_ASSERT(cell.type == lyric_runtime::DataCellType::I64);
    uint64_t timeout = cell.data.i64 > 0? cell.data.i64 : 0;

    auto *segmentManager = state->segmentManager();
    auto *heapManager = state->heapManager();

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

    // create a new future to wait for sleep result
    auto ref = heapManager->allocateRef<FutureRef>(vtable);
    currentCoro->pushData(ref);

    // register a timer
    auto promise = lyric_runtime::Promise::create(on_sleep_accept);
    scheduler->registerTimer(timeout, promise);

    // attach the timer promise to the future
    auto *fut = ref.data.ref;
    fut->prepareFuture(promise);

    return lyric_runtime::InterpreterStatus::ok();
}

static void
on_worker_complete(lyric_runtime::Promise *promise)
{
    auto *workerTask = static_cast<lyric_runtime::Task *>(promise->getData());

    // complete the promise
    auto *workerCoro = workerTask->stackfulCoroutine();
    auto result = workerCoro->peekData();
    TU_LOG_INFO << "worker task " << workerTask << " terminated with result " << result;
    promise->complete(result);

    // destroy the worker task
    auto *scheduler = workerTask->getSystemScheduler();
    TU_LOG_INFO << "destroying worker task " << workerTask;
    scheduler->destroyTask(workerTask);
}

tempo_utils::Status
std_system_spawn(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *scheduler = state->systemScheduler();
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &cell = frame.getArgument(0);
    TU_ASSERT(cell.type == lyric_runtime::DataCellType::REF);
    auto *closure = cell.data.ref;

    auto *segmentManager = state->segmentManager();
    auto *heapManager = state->heapManager();

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

    // create a new future to wait for spawn result
    auto ref = heapManager->allocateRef<FutureRef>(vtable);
    currentCoro->pushData(ref);
    auto *fut = ref.data.ref;

    // create a new worker task
    auto *workerTask = scheduler->createTask();
    TU_ASSERT (workerTask != nullptr);

    // push a new frame onto the worker task call stack
    if (!closure->applyClosure(workerTask, state))
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "failed to apply closure");

    // add the bottom stack guard
    workerTask->stackfulCoroutine()->pushGuard();

    //
    lyric_runtime::PromiseOptions options;
    options.data = workerTask;
    auto promise = lyric_runtime::Promise::create(on_worker_complete, options);

    // register a waiter bound to the current task
    scheduler->registerWorker(workerTask, promise);

    // attach the waiter to the future
    fut->prepareFuture(promise);

    // put worker task on the ready queue
    scheduler->resumeTask(workerTask);

    return lyric_runtime::InterpreterStatus::ok();
}
