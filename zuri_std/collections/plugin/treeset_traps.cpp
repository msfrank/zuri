/* SPDX-License-Identifier: BSD-3-Clause */

#include <tempo_utils/log_stream.h>

#include "treeset_ref.h"
#include "treeset_traps.h"

tempo_utils::Status
treeset_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();
    const auto *vtable = frame.getVirtualTable();
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<TreeSetRef>(vtable);
    currentCoro->pushData(ref);

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
treeset_ctor(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<TreeSetRef *>(receiver.data.ref);

    TU_ASSERT (frame.numArguments() == 1);
    const auto &ord = frame.getArgument(0);
    TU_ASSERT(ord.type == lyric_runtime::DataCellType::REF);
    const auto cmp = currentCoro->popData();
    TU_ASSERT(cmp.type == lyric_runtime::DataCellType::CALL);
    instance->initialize(TreeSetComparator(interp, state, ord, cmp));

    std::vector<lyric_runtime::DataCell> items;
    for (int i = 0; i < frame.numRest(); i++) {
        items.push_back(frame.getRest(i));
    }
    for (const auto &item : items) {
        instance->add(item);
    }

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
treeset_size(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<TreeSetRef *>(receiver.data.ref);
    currentCoro->pushData(lyric_runtime::DataCell(static_cast<int64_t>(instance->size())));
    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
treeset_contains(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &key = frame.getArgument(0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<TreeSetRef *>(receiver.data.ref);
    currentCoro->pushData(lyric_runtime::DataCell(instance->contains(key)));
    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
treeset_add(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &value = frame.getArgument(0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<TreeSetRef *>(receiver.data.ref);
    auto prev = instance->add(value);
    currentCoro->pushData(prev);
    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
treeset_remove(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &value = frame.getArgument(0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<TreeSetRef *>(receiver.data.ref);
    auto prev = instance->remove(value);
    currentCoro->pushData(prev);
    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
treeset_clear(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<TreeSetRef *>(receiver.data.ref);
    instance->clear();
    currentCoro->pushData(lyric_runtime::DataCell::nil());
    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
treeset_iterate(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    const auto cell = currentCoro->popData();
    TU_ASSERT(cell.type == lyric_runtime::DataCellType::CLASS);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<TreeSetRef *>(receiver.data.ref);

    lyric_runtime::InterpreterStatus status;
    const auto *vtable = state->segmentManager()->resolveClassVirtualTable(cell, status);
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<TreeSetIterator>(vtable, instance->begin(), instance);
    currentCoro->pushData(ref);

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
treeset_iterator_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();
    const auto *vtable = frame.getVirtualTable();
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<TreeSetIterator>(vtable);
    currentCoro->pushData(ref);

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
treeset_iterator_valid(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<lyric_runtime::AbstractRef *>(receiver.data.ref);
    currentCoro->pushData(lyric_runtime::DataCell(instance->iteratorValid()));

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
treeset_iterator_next(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<lyric_runtime::AbstractRef *>(receiver.data.ref);

    lyric_runtime::DataCell next;
    if (!instance->iteratorNext(next)) {
        next = lyric_runtime::DataCell();
    }
    currentCoro->pushData(next);

    return lyric_runtime::InterpreterStatus::ok();
}
