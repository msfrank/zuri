/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_TIME_COMPILE_INSTANT_H
#define ZURI_STD_TIME_COMPILE_INSTANT_H

#include <lyric_assembler/assembly_state.h>
#include <lyric_assembler/block_handle.h>

tempo_utils::Status
build_std_time_Instant(
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block);

#endif // ZURI_STD_TIME_COMPILE_INSTANT_H
