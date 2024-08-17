/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_COLLECTIONS_COMPILE_VECTOR_H
#define ZURI_STD_COLLECTIONS_COMPILE_VECTOR_H

#include <lyric_assembler/object_state.h>
#include <lyric_assembler/block_handle.h>
#include <lyric_typing/type_system.h>

tempo_utils::Result<lyric_assembler::ClassSymbol *>
declare_std_collections_Vector(
    lyric_assembler::ObjectState &state,
    lyric_assembler::BlockHandle *block);

tempo_utils::Status
build_std_collections_Vector(
    lyric_assembler::ClassSymbol *VectorClass,
    lyric_assembler::ClassSymbol *VectorIteratorClass,
    lyric_assembler::ObjectState &state,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem);

#endif // ZURI_STD_COLLECTIONS_COMPILE_VECTOR_H
