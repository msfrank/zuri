/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_TEXT_COMPILE_TEXT_H
#define ZURI_STD_TEXT_COMPILE_TEXT_H

#include <lyric_assembler/assembly_state.h>
#include <lyric_assembler/block_handle.h>

tempo_utils::Status
build_std_text_Text(
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block);

#endif // ZURI_STD_TEXT_COMPILE_TEXT_H
