/* SPDX-License-Identifier: BSD-3-Clause */

#include <tempo_utils/log_stream.h>

#include "treemap_ref.h"
#include "treemap_traps.h"

tempo_utils::Status
treemap_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();
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

    auto &frame = currentCoro->peekCall();
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<TreeMapRef *>(receiver.data.ref);

    TU_ASSERT (frame.numArguments() == 1);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT(arg0.type == lyric_runtime::DataCellType::REF);
    const auto cmp = currentCoro->popData();
    TU_ASSERT(cmp.type == lyric_runtime::DataCellType::CALL);
    instance->initialize(TreeMapComparator(interp, state, arg0, cmp));

    return {};
}

tempo_utils::Status
treemap_size(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

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

    auto &frame = currentCoro->peekCall();

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

    auto &frame = currentCoro->peekCall();

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

    auto &frame = currentCoro->peekCall();

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

    auto &frame = currentCoro->peekCall();

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

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<TreeMapRef *>(receiver.data.ref);
    instance->clear();
    currentCoro->pushData(lyric_runtime::DataCell::nil());
    return {};
}
