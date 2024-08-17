/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_SYSTEM_COMPILE_OPERATION_H
#define ZURI_STD_SYSTEM_COMPILE_OPERATION_H

#include <lyric_assembler/object_state.h>
#include <lyric_assembler/block_handle.h>
#include <lyric_typing/type_system.h>

tempo_utils::Result<lyric_assembler::StructSymbol *>
build_std_system_Operation(
    lyric_assembler::ObjectState &state,
    lyric_assembler::BlockHandle *block,
    lyric_assembler::StructSymbol *AttrStruct,
    lyric_typing::TypeSystem *typeSystem);

tempo_utils::Status
build_std_system_AppendOperation(
    lyric_assembler::StructSymbol *OperationStruct,
    lyric_assembler::ObjectState &state,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem);

tempo_utils::Status
build_std_system_InsertOperation(
    lyric_assembler::StructSymbol *OperationStruct,
    lyric_assembler::ObjectState &state,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem);

tempo_utils::Status
build_std_system_UpdateOperation(
    lyric_assembler::StructSymbol *OperationStruct,
    lyric_assembler::ObjectState &state,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem);

tempo_utils::Status
build_std_system_ReplaceOperation(
    lyric_assembler::StructSymbol *OperationStruct,
    lyric_assembler::ObjectState &state,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem);

tempo_utils::Status
build_std_system_EmitOperation(
    lyric_assembler::StructSymbol *OperationStruct,
    lyric_assembler::ObjectState &state,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem);

#endif // ZURI_STD_SYSTEM_COMPILE_OPERATION_H