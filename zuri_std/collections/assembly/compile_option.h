/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_COLLECTIONS_COMPILE_OPTION_H
#define ZURI_STD_COLLECTIONS_COMPILE_OPTION_H

#include <lyric_assembler/assembly_state.h>
#include <lyric_assembler/block_handle.h>
#include <lyric_compiler/module_entry.h>
#include <lyric_typing/type_system.h>

tempo_utils::Status
build_std_collections_Option(
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block,
    lyric_compiler::ModuleEntry &moduleEntry,
    lyric_typing::TypeSystem *typeSystem);

#endif // ZURI_STD_COLLECTIONS_COMPILE_OPTION_H
