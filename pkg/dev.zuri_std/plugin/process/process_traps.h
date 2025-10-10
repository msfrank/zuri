/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_PROCESS_PROCESS_TRAPS_H
#define ZURI_STD_PROCESS_PROCESS_TRAPS_H

#include <lyric_runtime/bytecode_interpreter.h>

tempo_utils::Status std_process_get_program_id(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status std_process_get_program_main(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status std_process_get_argument(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status std_process_num_arguments(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

#endif // ZURI_STD_PROCESS_PROCESS_TRAPS_H
