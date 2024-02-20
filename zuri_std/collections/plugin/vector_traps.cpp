/* SPDX-License-Identifier: BSD-3-Clause */

#include <tempo_utils/log_stream.h>

#include <lyric_runtime/interpreter_state.h>

#include "vector_ref.h"
#include "vector_traps.h"

tempo_utils::Status
vector_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();
    const auto *vtable = frame.getVirtualTable();
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<VectorRef>(vtable);
    currentCoro->pushData(ref);

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
vector_ctor(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver != nullptr);
    auto *instance = static_cast<VectorRef *>(receiver);

    for (int i = 0; i < frame.numRest(); i++) {
        const auto &item = frame.getRest(i);
        instance->append(item);
    }

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
vector_size(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver != nullptr);
    auto *instance = static_cast<VectorRef *>(receiver);
    currentCoro->pushData(lyric_runtime::DataCell(static_cast<int64_t>(instance->size())));
    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
vector_at(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &idx = frame.getArgument(0);
    TU_ASSERT(idx.type == lyric_runtime::DataCellType::I64);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver != nullptr);
    auto *instance = static_cast<VectorRef *>(receiver);
    currentCoro->pushData(instance->at(idx.data.i64));
    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
vector_append(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &val = frame.getArgument(0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver != nullptr);
    auto *instance = static_cast<VectorRef *>(receiver);
    instance->append(val);
    currentCoro->pushData(lyric_runtime::DataCell::nil());
    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
vector_insert(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 2);
    const auto &idx = frame.getArgument(0);
    TU_ASSERT(idx.type == lyric_runtime::DataCellType::I64);
    const auto &val = frame.getArgument(1);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver != nullptr);
    auto *instance = static_cast<VectorRef *>(receiver);
    instance->insert(idx.data.i64, val);
    currentCoro->pushData(lyric_runtime::DataCell::nil());
    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
vector_update(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 2);
    const auto &idx = frame.getArgument(0);
    TU_ASSERT(idx.type == lyric_runtime::DataCellType::I64);
    const auto &val = frame.getArgument(1);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver != nullptr);
    auto *instance = static_cast<VectorRef *>(receiver);
    currentCoro->pushData(instance->update(idx.data.i64, val));
    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
vector_remove(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &idx = frame.getArgument(0);
    TU_ASSERT(idx.type == lyric_runtime::DataCellType::I64);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver != nullptr);
    auto *instance = static_cast<VectorRef *>(receiver);
    currentCoro->pushData(instance->remove(idx.data.i64));
    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
vector_clear(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver != nullptr);
    auto *instance = static_cast<VectorRef *>(receiver);
    instance->clear();
    currentCoro->pushData(lyric_runtime::DataCell::nil());
    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
vector_iter(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    const auto cell = currentCoro->popData();
    TU_ASSERT(cell.type == lyric_runtime::DataCellType::CLASS);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver != nullptr);
    auto *instance = static_cast<VectorRef *>(receiver);

    lyric_runtime::InterpreterStatus status;
    const auto *vtable = state->segmentManager()->resolveClassVirtualTable(cell, status);
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<VectorIterator>(vtable, instance->begin(), instance);
    currentCoro->pushData(ref);

    return lyric_runtime::InterpreterStatus::ok();
}
