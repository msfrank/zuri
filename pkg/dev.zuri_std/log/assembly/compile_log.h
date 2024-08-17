/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_LOG_COMPILE_LOG_H
#define ZURI_STD_LOG_COMPILE_LOG_H

#include <lyric_assembler/block_handle.h>
#include <lyric_assembler/object_state.h>

tempo_utils::Status
build_std_log(
    lyric_assembler::ObjectState &state,
    lyric_assembler::BlockHandle *block);

#endif // ZURI_STD_LOG_COMPILE_LOG_H
