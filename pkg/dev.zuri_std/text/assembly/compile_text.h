/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_TEXT_COMPILE_TEXT_H
#define ZURI_STD_TEXT_COMPILE_TEXT_H

#include <lyric_assembler/object_state.h>
#include <lyric_assembler/block_handle.h>

tempo_utils::Status
build_std_text_Text(
    lyric_assembler::ObjectState &state,
    lyric_assembler::NamespaceSymbol *globalNamespace);

#endif // ZURI_STD_TEXT_COMPILE_TEXT_H
