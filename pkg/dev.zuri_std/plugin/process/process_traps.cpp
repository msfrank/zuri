/* SPDX-License-Identifier: BSD-3-Clause */

#include <tempo_utils/log_stream.h>
#include <tempo_utils/uuid.h>

#include "process_traps.h"

tempo_utils::Status
std_process_get_program_id(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state)
{
    auto *heapManager = state->heapManager();
    auto *currentCoro = state->currentCoro();

    auto uuid = tempo_utils::UUID::randomUUID();
    auto programId = heapManager->allocateString(uuid.toString());
    currentCoro->pushData(programId);

    return {};
}

tempo_utils::Status
std_process_get_program_main(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state)
{
    auto *heapManager = state->heapManager();
    auto *currentCoro = state->currentCoro();

    auto mainLocation = state->getMainLocation().toUrl();
    auto programMain = heapManager->allocateUrl(mainLocation);
    currentCoro->pushData(programMain);

    return {};
}

tempo_utils::Status
std_process_get_argument(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &idx = frame.getArgument(0);
    TU_ASSERT(idx.type == lyric_runtime::DataCellType::I64);

    auto argument = state->getMainArgument(idx.data.i64);
    currentCoro->pushData(argument);

    return {};
}

tempo_utils::Status
std_process_num_arguments(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto numArguments = lyric_runtime::DataCell(static_cast<tu_int64>(state->numMainArguments()));
    currentCoro->pushData(numArguments);

    return {};
}
