/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_SYSTEM_COMPILE_ELEMENT_H
#define ZURI_STD_SYSTEM_COMPILE_ELEMENT_H

#include <lyric_assembler/assembly_state.h>
#include <lyric_assembler/block_handle.h>
#include <lyric_typing/type_system.h>

tempo_utils::Result<lyric_assembler::StructSymbol *>
declare_std_system_Element(
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block);

tempo_utils::Status
build_std_system_Element(
    lyric_assembler::StructSymbol *ElementStruct,
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem);

#endif // ZURI_STD_SYSTEM_COMPILE_ELEMENT_H