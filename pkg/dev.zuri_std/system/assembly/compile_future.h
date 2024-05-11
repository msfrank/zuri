/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_SYSTEM_COMPILE_FUTURE_H
#define ZURI_STD_SYSTEM_COMPILE_FUTURE_H

#include <lyric_assembler/assembly_state.h>
#include <lyric_assembler/block_handle.h>
#include <lyric_compiler/module_entry.h>

tempo_utils::Status
build_std_system_Future(
    lyric_compiler::ModuleEntry &moduleEntry,
    lyric_assembler::BlockHandle *block);

#endif // ZURI_STD_SYSTEM_COMPILE_FUTURE_H