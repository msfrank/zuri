/* SPDX-License-Identifier: BSD-3-Clause */

#include <tempo_utils/log_stream.h>

#include "hashmap_ref.h"
#include "hashmap_traps.h"

tempo_utils::Status
hashmap_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    TU_ASSERT(vtable != nullptr);
    auto *currentCoro = state->currentCoro();
    auto ref = state->heapManager()->allocateRef<HashMapRef>(vtable);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(ref));
    return {};
}

tempo_utils::Status
hashmap_ctor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    HashMapRef *instance;
    TU_ASSERT(receiver.castRef(instance));

    TU_ASSERT (frame.numArguments() == 1);
    const auto &ctx = frame.getArgument(0);

    lyric_runtime::Operand eq;
    TU_RETURN_IF_NOT_OK (currentCoro->popData(eq));

    instance->initialize(HashMapEq(interp, state, ctx, eq));

    return {};
}

tempo_utils::Status
hashmap_size(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    HashMapRef *instance;
    TU_ASSERT(receiver.castRef(instance));
    auto size = lyric_runtime::Operand::fromI64(static_cast<int64_t>(instance->size()));
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(size));
    return {};
}

tempo_utils::Status
hashmap_contains(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &key = frame.getArgument(0);

    auto receiver = frame.getReceiver();
    HashMapRef *instance;
    TU_ASSERT(receiver.castRef(instance));
    auto contains = lyric_runtime::Operand::fromBool(instance->contains(key));
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(contains));
    return {};
}

tempo_utils::Status
hashmap_get(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &key = frame.getArgument(0);

    auto receiver = frame.getReceiver();
    HashMapRef *instance;
    TU_ASSERT(receiver.castRef(instance));
    auto value = instance->get(key);
    if (!value.isValid()) {
        value = lyric_runtime::Operand::undef();
    }
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(value));
    return {};
}

tempo_utils::Status
hashmap_put(
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
    HashMapRef *instance;
    TU_ASSERT(receiver.castRef(instance));
    auto prev = instance->put(key, val);
    if (!prev.isValid()) {
        prev = lyric_runtime::Operand::undef();
    }
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(prev));
    return {};
}

tempo_utils::Status
hashmap_remove(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &key = frame.getArgument(0);

    auto receiver = frame.getReceiver();
    HashMapRef *instance;
    TU_ASSERT(receiver.castRef(instance));
    auto prev = instance->remove(key);
    if (!prev.isValid()) {
        prev = lyric_runtime::Operand::undef();
    }
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(prev));
    return {};
}

tempo_utils::Status
hashmap_clear(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    HashMapRef *instance;
    TU_ASSERT(receiver.castRef(instance));
    instance->clear();
    return {};
}

tempo_utils::Status
hashmap_iterate(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *unused)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    lyric_runtime::Operand cell;
    TU_RETURN_IF_NOT_OK (currentCoro->popData(cell));

    auto receiver = frame.getReceiver();
    HashMapRef *instance;
    TU_ASSERT(receiver.castRef(instance));

    lyric_runtime::InterpreterStatus status;
    const auto *vtable = state->segmentManager()->resolveClassVirtualTable(cell, status);
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<HashMapIterator>(vtable, instance);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(ref));
    return {};
}

tempo_utils::Status
hashmap_iterator_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    TU_ASSERT(vtable != nullptr);
    auto *currentCoro = state->currentCoro();
    auto ref = state->heapManager()->allocateRef<HashMapIterator>(vtable);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(ref));
    return {};
}

tempo_utils::Status
hashmap_iterator_valid(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    HashMapIterator *instance;
    TU_ASSERT(receiver.castRef(instance));
    auto valid = lyric_runtime::Operand::fromBool(instance->valid());
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(valid));

    return {};
}

tempo_utils::Status
hashmap_iterator_get_key(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    HashMapIterator *instance;
    TU_ASSERT(receiver.castRef(instance));
    auto key = instance->key();
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(key));

    return {};
}

tempo_utils::Status
hashmap_iterator_get_value(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    HashMapIterator *instance;
    TU_ASSERT(receiver.castRef(instance));
    auto value = instance->value();
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(value));

    return {};
}

tempo_utils::Status
hashmap_iterator_next(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    HashMapIterator *instance;
    TU_ASSERT(receiver.castRef(instance));
    instance->next();

    return {};
}
