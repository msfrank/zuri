/* SPDX-License-Identifier: BSD-3-Clause */

#include <iostream>

#include <lyric_runtime/data_cell.h>
#include <lyric_runtime/interpreter_state.h>

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
    TU_ASSERT(cell.type == lyric_runtime::DataCellType::REF);

    tempo_utils::Url protocolUrl;
    if (!cell.data.ref->uriValue(protocolUrl))
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

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() >= 1);
    const auto &cell = frame.getArgument(0);
    TU_ASSERT(cell.type == lyric_runtime::DataCellType::REF);
    auto *future = cell.data.ref;

    // consume the waiter if it exists
    lyric_runtime::Waiter *waiter = nullptr;

    if (future->releaseWaiter(&waiter)) {
        if (waiter->handle) {
            // if the future has not been awaited and has not completed, then suspend the current task
            auto *scheduler = state->systemScheduler();
            waiter->task = scheduler->currentTask();
            TU_LOG_INFO << "suspending task " << waiter->task;
            scheduler->suspendTask(waiter->task);
        }
    }

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
    auto *future = cell.data.ref;

    lyric_runtime::DataCell result;
    future->resolveFuture(result, interp, state);
    currentCoro->pushData(result);

    return lyric_runtime::InterpreterStatus::ok();
}

static void
on_sleep_complete(lyric_runtime::Waiter *waiter, void *data)
{
    auto *future = (FutureRef *) data;
    future->complete(lyric_runtime::DataCell::nil());
}

tempo_utils::Status
std_system_sleep(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
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

    // register a waiter bound to the current task
    auto *fut = static_cast<FutureRef *>(ref.data.ref);
    auto scheduler = state->systemScheduler();
    auto *waiter = scheduler->registerTimer(timeout, on_sleep_complete, fut);

    // attach the waiter to the future
    fut->attachWaiter(waiter);

    return lyric_runtime::InterpreterStatus::ok();
}
