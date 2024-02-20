/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_SYSTEM_COMPILE_PORT_H
#define ZURI_STD_SYSTEM_COMPILE_PORT_H

#include <lyric_assembler/assembly_state.h>
#include <lyric_assembler/block_handle.h>
#include <lyric_assembler/struct_symbol.h>

tempo_utils::Status
build_std_system_Port(
    lyric_assembler::StructSymbol *OperationStruct,
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block);

#endif // ZURI_STD_SYSTEM_COMPILE_PORT_H