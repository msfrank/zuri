/* SPDX-License-Identifier: BSD-3-Clause */

#include <tempo_utils/log_stream.h>

#include "treemap_ref.h"
#include "treemap_traps.h"

tempo_utils::Status
treemap_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    TU_ASSERT(vtable != nullptr);
    auto *currentCoro = state->currentCoro();
    auto ref = state->heapManager()->allocateRef<TreeMapRef>(vtable);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(ref));
    return {};
}

tempo_utils::Status
treemap_ctor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    TreeMapRef *instance;
    TU_ASSERT(receiver.castRef(instance));

    TU_ASSERT (frame.numArguments() == 1);
    const auto &ctx = frame.getArgument(0);

    lyric_runtime::Operand cmp;
    TU_RETURN_IF_NOT_OK (currentCoro->popData(cmp));
    instance->initialize(TreeMapComparator(interp, state, ctx, cmp));

    return {};
}

tempo_utils::Status
treemap_size(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TreeMapRef *instance;
    TU_ASSERT(receiver.castRef(instance));
    auto size = lyric_runtime::Operand::fromI64(static_cast<tu_int64>(instance->size()));
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(size));
    return {};
}

tempo_utils::Status
treemap_contains(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &key = frame.getArgument(0);

    auto receiver = frame.getReceiver();
    TreeMapRef *instance;
    TU_ASSERT(receiver.castRef(instance));
    auto contains = lyric_runtime::Operand::fromBool(instance->contains(key));
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(contains));
    return {};
}

tempo_utils::Status
treemap_get(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &key = frame.getArgument(0);

    auto receiver = frame.getReceiver();
    TreeMapRef *instance;
    TU_ASSERT(receiver.castRef(instance));
    auto value = instance->get(key);
    if (!value.isValid()) {
        value = lyric_runtime::Operand::undef();
    }
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(value));
    return {};
}

tempo_utils::Status
treemap_put(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 2);
    const auto &key = frame.getArgument(0);
    const auto &val = frame.getArgument(1);

    auto receiver = frame.getReceiver();
    TreeMapRef *instance;
    TU_ASSERT(receiver.castRef(instance));
    auto prev = instance->put(key, val);
    if (!prev.isValid()) {
        prev = lyric_runtime::Operand::undef();
    }
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(prev));
    return {};
}

tempo_utils::Status
treemap_remove(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &key = frame.getArgument(0);

    auto receiver = frame.getReceiver();
    TreeMapRef *instance;
    TU_ASSERT(receiver.castRef(instance));
    auto prev = instance->remove(key);
    if (!prev.isValid()) {
        prev = lyric_runtime::Operand::undef();
    }
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(prev));
    return {};
}

tempo_utils::Status
treemap_clear(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TreeMapRef *instance;
    TU_ASSERT(receiver.castRef(instance));
    instance->clear();
    return {};
}

tempo_utils::Status
treemap_iterate(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *unused)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    lyric_runtime::Operand cell;
    TU_RETURN_IF_NOT_OK (currentCoro->popData(cell));

    auto receiver = frame.getReceiver();
    TreeMapRef *instance;
    TU_ASSERT(receiver.castRef(instance));

    lyric_runtime::InterpreterStatus status;
    const auto *vtable = state->segmentManager()->resolveClassVirtualTable(cell, status);
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<TreeMapIterator>(vtable, instance);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(ref));
    return {};
}

tempo_utils::Status
treemap_iterator_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    TU_ASSERT(vtable != nullptr);
    auto *currentCoro = state->currentCoro();
    auto ref = state->heapManager()->allocateRef<TreeMapIterator>(vtable);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(ref));
    return {};
}

tempo_utils::Status
treemap_iterator_valid(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TreeMapIterator *instance;
    TU_ASSERT(receiver.castRef(instance));
    auto valid = lyric_runtime::Operand::fromBool(instance->valid());
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(valid));
    return {};
}

tempo_utils::Status
treemap_iterator_get_key(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TreeMapIterator *instance;
    TU_ASSERT(receiver.castRef(instance));
    auto key = instance->key();
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(key));
    return {};
}

tempo_utils::Status
treemap_iterator_get_value(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TreeMapIterator *instance;
    TU_ASSERT(receiver.castRef(instance));
    auto value = instance->value();
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(value));
    return {};
}

tempo_utils::Status
treemap_iterator_next(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TreeMapIterator *instance;
    TU_ASSERT(receiver.castRef(instance));
    instance->next();
    return {};
}
