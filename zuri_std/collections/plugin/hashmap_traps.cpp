/* SPDX-License-Identifier: BSD-3-Clause */

#include <tempo_utils/log_stream.h>

#include "hashmap_ref.h"
#include "hashmap_traps.h"

tempo_utils::Status
hashmap_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();
    const auto *vtable = frame.getVirtualTable();
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<HashMapRef>(vtable);
    currentCoro->pushData(ref);

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
hashmap_ctor(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver != nullptr);
    auto *instance = static_cast<HashMapRef *>(receiver);

    TU_ASSERT (frame.numArguments() == 1);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT(arg0.type == lyric_runtime::DataCellType::REF);
    const auto cmp = currentCoro->popData();
    TU_ASSERT(cmp.type == lyric_runtime::DataCellType::CALL);
    instance->initialize(HashMapEq(interp, state, arg0, cmp));

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
hashmap_size(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver != nullptr);
    auto *instance = static_cast<HashMapRef *>(receiver);
    currentCoro->pushData(lyric_runtime::DataCell(static_cast<int64_t>(instance->hashSize())));
    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
hashmap_contains(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &key = frame.getArgument(0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver != nullptr);
    auto *instance = static_cast<HashMapRef *>(receiver);
    currentCoro->pushData(lyric_runtime::DataCell(instance->hashContains(key)));
    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
hashmap_get(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &key = frame.getArgument(0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver != nullptr);
    auto *instance = static_cast<HashMapRef *>(receiver);
    currentCoro->pushData(instance->hashGet(key));
    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
hashmap_put(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 2);
    const auto &key = frame.getArgument(0);
    const auto &val = frame.getArgument(1);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver != nullptr);
    auto *instance = static_cast<HashMapRef *>(receiver);
    auto prev = instance->hashPut(key, val);
    currentCoro->pushData(prev);
    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
hashmap_remove(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &key = frame.getArgument(0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver != nullptr);
    auto *instance = static_cast<HashMapRef *>(receiver);
    auto prev = instance->hashRemove(key);
    currentCoro->pushData(prev);
    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
hashmap_clear(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver != nullptr);
    auto *instance = static_cast<HashMapRef *>(receiver);
    instance->hashClear();
    currentCoro->pushData(lyric_runtime::DataCell::nil());
    return lyric_runtime::InterpreterStatus::ok();
}
