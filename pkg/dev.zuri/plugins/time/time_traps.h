/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_TIME_TIME_TRAPS_H
#define ZURI_STD_TIME_TIME_TRAPS_H

#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/call_cell.h>
#include <lyric_runtime/data_cell.h>
#include <lyric_runtime/interpreter_state.h>

tempo_utils::Status std_time_now(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status std_time_parse_timezone(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

#endif // ZURI_STD_TIME_TIME_TRAPS_H