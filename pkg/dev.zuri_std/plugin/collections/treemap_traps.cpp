/* SPDX-License-Identifier: BSD-3-Clause */

#include <tempo_utils/log_stream.h>

#include "treemap_ref.h"
#include "treemap_traps.h"

tempo_utils::Status
treemap_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    const auto *vtable = frame.getVirtualTable();
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<TreeMapRef>(vtable);
    currentCoro->pushData(ref);

    return {};
}

tempo_utils::Status
treemap_ctor(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<TreeMapRef *>(receiver.data.ref);

    TU_ASSERT (frame.numArguments() == 1);
    const auto &ctxArgument = frame.getArgument(0);
    TU_ASSERT(ctxArgument.type == lyric_runtime::DataCellType::REF);

    lyric_runtime::DataCell compareCall;
    TU_RETURN_IF_NOT_OK (currentCoro->popData(compareCall));
    TU_ASSERT(compareCall.type == lyric_runtime::DataCellType::CALL);
    instance->initialize(TreeMapComparator(interp, state, ctxArgument, compareCall));

    return {};
}

tempo_utils::Status
treemap_size(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<TreeMapRef *>(receiver.data.ref);
    currentCoro->pushData(lyric_runtime::DataCell(static_cast<int64_t>(instance->size())));
    return {};
}

tempo_utils::Status
treemap_contains(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &key = frame.getArgument(0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<TreeMapRef *>(receiver.data.ref);
    currentCoro->pushData(lyric_runtime::DataCell(instance->contains(key)));
    return {};
}

tempo_utils::Status
treemap_get(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &key = frame.getArgument(0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<TreeMapRef *>(receiver.data.ref);
    currentCoro->pushData(instance->get(key));
    return {};
}

tempo_utils::Status
treemap_put(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 2);
    const auto &key = frame.getArgument(0);
    const auto &val = frame.getArgument(1);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<TreeMapRef *>(receiver.data.ref);
    auto prev = instance->put(key, val);
    currentCoro->pushData(prev);
    return {};
}

tempo_utils::Status
treemap_remove(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &key = frame.getArgument(0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<TreeMapRef *>(receiver.data.ref);
    auto prev = instance->remove(key);
    currentCoro->pushData(prev);
    return {};
}

tempo_utils::Status
treemap_clear(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<TreeMapRef *>(receiver.data.ref);
    instance->clear();
    return {};
}

tempo_utils::Status
treemap_iterate(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    lyric_runtime::DataCell cell;
    TU_RETURN_IF_NOT_OK (currentCoro->popData(cell));
    TU_ASSERT(cell.type == lyric_runtime::DataCellType::CLASS);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<TreeMapRef *>(receiver.data.ref);

    lyric_runtime::InterpreterStatus status;
    const auto *vtable = state->segmentManager()->resolveClassVirtualTable(cell, status);
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<TreeMapIterator>(vtable, instance);
    currentCoro->pushData(ref);

    return {};
}

tempo_utils::Status
treemap_iterator_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    const auto *vtable = frame.getVirtualTable();
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<TreeMapIterator>(vtable);
    currentCoro->pushData(ref);

    return {};
}

tempo_utils::Status
treemap_iterator_valid(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<TreeMapIterator *>(receiver.data.ref);
    currentCoro->pushData(lyric_runtime::DataCell(instance->valid()));

    return {};
}

tempo_utils::Status
treemap_iterator_get_key(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<TreeMapIterator *>(receiver.data.ref);
    currentCoro->pushData(instance->key());

    return {};
}

tempo_utils::Status
treemap_iterator_get_value(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<TreeMapIterator *>(receiver.data.ref);
    currentCoro->pushData(instance->value());

    return {};
}

tempo_utils::Status
treemap_iterator_next(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<TreeMapIterator *>(receiver.data.ref);
    instance->next();

    return {};
}
