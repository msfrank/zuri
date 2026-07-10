/* SPDX-License-Identifier: BSD-3-Clause */

#include <tempo_utils/log_stream.h>

#include "treeset_ref.h"
#include "treeset_traps.h"

tempo_utils::Status
treeset_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    TU_ASSERT(vtable != nullptr);
    auto *currentCoro = state->currentCoro();
    auto ref = state->heapManager()->allocateRef<TreeSetRef>(vtable);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(ref));
    return {};
}

tempo_utils::Status
treeset_ctor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    TreeSetRef *instance;
    TU_ASSERT(receiver.castRef(instance));

    TU_ASSERT (frame.numArguments() == 1);
    const auto &ctx = frame.getArgument(0);

    lyric_runtime::Operand cmp;
    TU_RETURN_IF_NOT_OK (currentCoro->popData(cmp));
    instance->initialize(TreeSetComparator(interp, state, ctx, cmp));
    return {};
}

tempo_utils::Status
treeset_size(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TreeSetRef *instance;
    TU_ASSERT(receiver.castRef(instance));
    auto size = lyric_runtime::Operand::fromI64(static_cast<int64_t>(instance->size()));
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(size));
    return {};
}

tempo_utils::Status
treeset_contains(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &key = frame.getArgument(0);

    auto receiver = frame.getReceiver();
    TreeSetRef *instance;
    TU_ASSERT(receiver.castRef(instance));
    auto contains = lyric_runtime::Operand::fromBool(instance->contains(key));
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(contains));
    return {};
}

tempo_utils::Status
treeset_add(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &value = frame.getArgument(0);

    auto receiver = frame.getReceiver();
    TreeSetRef *instance;
    TU_ASSERT(receiver.castRef(instance));
    auto added = instance->add(value);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(added));
    return {};
}

tempo_utils::Status
treeset_remove(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &value = frame.getArgument(0);

    auto receiver = frame.getReceiver();
    TreeSetRef *instance;
    TU_ASSERT(receiver.castRef(instance));
    auto removed = instance->remove(value);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(removed));
    return {};
}

tempo_utils::Status
treeset_replace(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &value = frame.getArgument(0);

    auto receiver = frame.getReceiver();
    TreeSetRef *instance;
    TU_ASSERT(receiver.castRef(instance));
    auto replaced = instance->replace(value);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(replaced));
    return {};
}

tempo_utils::Status
treeset_clear(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TreeSetRef *instance;
    TU_ASSERT(receiver.castRef(instance));
    instance->clear();
    return {};
}

tempo_utils::Status
treeset_iterate(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *unused)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    lyric_runtime::Operand cell;
    TU_RETURN_IF_NOT_OK (currentCoro->popData(cell));

    auto receiver = frame.getReceiver();
    TreeSetRef *instance;
    TU_ASSERT(receiver.castRef(instance));

    lyric_runtime::InterpreterStatus status;
    const auto *vtable = state->segmentManager()->resolveClassVirtualTable(cell, status);
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<TreeSetIterator>(vtable, instance);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(ref));
    return {};
}

tempo_utils::Status
treeset_iterator_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    TU_ASSERT(vtable != nullptr);
    auto *currentCoro = state->currentCoro();
    auto ref = state->heapManager()->allocateRef<TreeSetIterator>(vtable);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(ref));
    return {};
}

tempo_utils::Status
treeset_iterator_valid(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TreeSetIterator *instance;
    TU_ASSERT(receiver.castRef(instance));
    auto valid = lyric_runtime::Operand::fromBool(instance->iteratorValid());
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(valid));
    return {};
}

tempo_utils::Status
treeset_iterator_next(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TreeSetIterator *instance;
    TU_ASSERT(receiver.castRef(instance));

    lyric_runtime::Operand next;
    if (!instance->iteratorNext(next)) {
        next = lyric_runtime::Operand();
    }
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(next));

    return {};
}
