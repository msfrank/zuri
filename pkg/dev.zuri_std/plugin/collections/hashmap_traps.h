/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_COLLECTIONS_HASHMAP_TRAPS_H
#define ZURI_STD_COLLECTIONS_HASHMAP_TRAPS_H

#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/interpreter_state.h>

tempo_utils::Status hashmap_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status hashmap_ctor(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status hashmap_size(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status hashmap_contains(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status hashmap_get(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status hashmap_put(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status hashmap_remove(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status hashmap_clear(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);

#endif // ZURI_STD_COLLECTIONS_HASHMAP_TRAPS_H
