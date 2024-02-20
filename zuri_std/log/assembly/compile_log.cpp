/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <zuri_std_log/lib_types.h>

#include "compile_log.h"

tempo_utils::Status
build_std_log(
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block)
{
    auto *symbolCache = state.symbolCache();
    {
        auto StringSpec = lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"String"}));
        auto BoolSpec = lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"Bool"}));
        auto declareFunctionResult = block->declareFunction("log",
            {
                { {}, "message", "", StringSpec, lyric_parser::BindingType::VALUE },
            },
            {},
            {},
            BoolSpec,
            lyric_object::AccessType::Public,
            {});
        auto functionUrl = declareFunctionResult.getResult();
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(functionUrl));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<uint32_t>(StdLogTrap::LOG));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return lyric_assembler::AssemblerStatus::ok();
}