/* SPDX-License-Identifier: BSD-3-Clause */

#include <tempo_utils/log_stream.h>

#include <lyric_runtime/interpreter_state.h>

#include "vector_ref.h"
#include "vector_traps.h"

tempo_utils::Status
vector_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    TU_ASSERT(vtable != nullptr);
    auto *currentCoro = state->currentCoro();
    auto ref = state->heapManager()->allocateRef<VectorRef>(vtable);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(ref));
    return {};
}

tempo_utils::Status
vector_ctor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.getType() == lyric_runtime::OperandType::Ref);
    return {};
}

tempo_utils::Status
vector_size(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    VectorRef *instance;
    TU_ASSERT(receiver.castRef(instance));
    auto size = lyric_runtime::Operand::fromI64(static_cast<tu_int64>(instance->size()));
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(size));
    return {};
}

tempo_utils::Status
vector_at(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &arg0 = frame.getArgument(0);
    tu_int64 index;
    TU_ASSERT(arg0.getI64(index));

    auto receiver = frame.getReceiver();
    VectorRef *instance;
    TU_ASSERT(receiver.castRef(instance));
    auto element = instance->at(index);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(element));
    return {};
}

tempo_utils::Status
vector_append(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &arg0 = frame.getArgument(0);

    auto receiver = frame.getReceiver();
    VectorRef *instance;
    TU_ASSERT(receiver.castRef(instance));
    instance->append(arg0);
    return {};
}

tempo_utils::Status
vector_insert(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 2);
    const auto &arg0 = frame.getArgument(0);
    tu_int64 index;
    TU_ASSERT(arg0.getI64(index));
    const auto &val = frame.getArgument(1);

    auto receiver = frame.getReceiver();
    VectorRef *instance;
    TU_ASSERT(receiver.castRef(instance));
    instance->insert(index, val);
    return {};
}

tempo_utils::Status
vector_replace(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 2);
    const auto &arg0 = frame.getArgument(0);
    tu_int64 index;
    TU_ASSERT(arg0.getI64(index));
    const auto &val = frame.getArgument(1);

    auto receiver = frame.getReceiver();
    VectorRef *instance;
    TU_ASSERT(receiver.castRef(instance));
    auto updated = instance->update(index, val);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(updated));
    return {};
}

tempo_utils::Status
vector_remove(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &arg0 = frame.getArgument(0);
    tu_int64 index;
    TU_ASSERT(arg0.getI64(index));

    auto receiver = frame.getReceiver();
    VectorRef *instance;
    TU_ASSERT(receiver.castRef(instance));
    auto removed = instance->remove(index);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(removed));
    return {};
}

tempo_utils::Status
vector_clear(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    VectorRef *instance;
    TU_ASSERT(receiver.castRef(instance));
    instance->clear();
    return {};
}

tempo_utils::Status
vector_iterate(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *unused)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    lyric_runtime::Operand cell;
    TU_RETURN_IF_NOT_OK (currentCoro->popData(cell));

    auto receiver = frame.getReceiver();
    VectorRef *instance;
    TU_ASSERT(receiver.castRef(instance));

    lyric_runtime::InterpreterStatus status;
    const auto *vtable = state->segmentManager()->resolveClassVirtualTable(cell, status);
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<VectorIterator>(vtable, instance);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(ref));
    return {};
}

tempo_utils::Status
vector_iterator_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    TU_ASSERT(vtable != nullptr);
    auto *currentCoro = state->currentCoro();
    auto ref = state->heapManager()->allocateRef<VectorIterator>(vtable);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(ref));
    return {};
}

tempo_utils::Status
vector_iterator_valid(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    VectorIterator *instance;
    TU_ASSERT(receiver.castRef(instance));
    auto valid = lyric_runtime::Operand::fromBool(instance->iteratorValid());
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(valid));
    return {};
}

tempo_utils::Status
vector_iterator_next(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    VectorIterator *instance;
    TU_ASSERT(receiver.castRef(instance));

    lyric_runtime::Operand next;
    if (!instance->iteratorNext(next)) {
        next = lyric_runtime::Operand();
    }
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(next));

    return {};
}
