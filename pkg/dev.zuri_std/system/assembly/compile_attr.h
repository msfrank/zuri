/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_SYSTEM_COMPILE_ATTR_H
#define ZURI_STD_SYSTEM_COMPILE_ATTR_H

#include <lyric_assembler/object_state.h>
#include <lyric_assembler/block_handle.h>

tempo_utils::Result<lyric_assembler::StructSymbol *>
declare_std_system_Attr(
    lyric_assembler::ObjectState &state,
    lyric_assembler::BlockHandle *block);

tempo_utils::Status
build_std_system_Attr(
    lyric_assembler::StructSymbol *AttrStruct,
    lyric_assembler::ObjectState &state,
    lyric_assembler::BlockHandle *block);

#endif // ZURI_STD_SYSTEM_COMPILE_ATTR_H