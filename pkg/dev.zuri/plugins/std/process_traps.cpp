/* SPDX-License-Identifier: BSD-3-Clause */

#include <tempo_utils/log_stream.h>
#include <tempo_utils/uuid.h>

#include "process_traps.h"

tempo_utils::Status
std_process_get_program_id(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *heapManager = state->heapManager();
    auto *currentCoro = state->currentCoro();

    auto uuid = tempo_utils::UUID::randomUUID();
    auto programId = heapManager->allocateString(uuid.toRfc4122String(), /* permanent= */ false);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(programId));

    return {};
}

tempo_utils::Status
std_process_get_program_main(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *heapManager = state->heapManager();

    auto mainLocation = state->getMainLocation().toString();
    TU_RETURN_IF_NOT_OK (heapManager->loadStringOntoStack(mainLocation));

    return {};
}

tempo_utils::Status
std_process_get_argument(
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

    auto argument = state->getMainArgument(index);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(argument));

    return {};
}

tempo_utils::Status
std_process_num_arguments(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto numArguments = lyric_runtime::Operand::fromI64(state->numMainArguments());
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(numArguments));

    return {};
}
