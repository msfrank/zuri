/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_TIME_COMPILE_TIMEZONE_H
#define ZURI_STD_TIME_COMPILE_TIMEZONE_H

#include <lyric_assembler/object_state.h>
#include <lyric_assembler/block_handle.h>

tempo_utils::Status
build_std_time_Timezone(
    lyric_assembler::ObjectState &state,
    lyric_assembler::BlockHandle *block);

#endif // ZURI_STD_TIME_COMPILE_TIMEZONE_H
