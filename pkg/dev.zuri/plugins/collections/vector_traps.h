/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_COLLECTIONS_VECTOR_TRAPS_H
#define ZURI_STD_COLLECTIONS_VECTOR_TRAPS_H

#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/interpreter_state.h>

tempo_utils::Status vector_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status vector_ctor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status vector_size(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status vector_at(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status vector_insert(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status vector_append(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status vector_replace(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status vector_remove(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status vector_clear(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status vector_iterate(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status vector_iterator_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status vector_iterator_next(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status vector_iterator_valid(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

#endif // ZURI_STD_COLLECTIONS_VECTOR_TRAPS_H