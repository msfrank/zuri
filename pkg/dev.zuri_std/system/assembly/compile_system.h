/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_SYSTEM_COMPILE_SYSTEM_H
#define ZURI_STD_SYSTEM_COMPILE_SYSTEM_H

#include <lyric_assembler/object_state.h>
#include <lyric_assembler/block_handle.h>
#include <lyric_typing/type_system.h>

tempo_utils::Status build_std_system(
    lyric_assembler::ObjectState &state,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem);

#endif // ZURI_STD_SYSTEM_COMPILE_SYSTEM_H
