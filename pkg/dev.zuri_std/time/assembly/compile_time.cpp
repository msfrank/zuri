/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_typing/callsite_reifier.h>
#include <zuri_std_time/lib_types.h>

#include "compile_time.h"

tempo_utils::Status
build_std_time(
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem)
{
    auto *fundamentalCache = state.fundamentalCache();
    auto *symbolCache = state.symbolCache();

    auto StringSpec = lyric_parser::Assignable::forSingular(
        fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::String));
    auto InstantSpec = lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"Instant"}));
    auto TimezoneSpec = lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"Timezone"}));

    {
        auto declareFunctionResult = block->declareFunction("Now",
            {},
            {},
            {},
            InstantSpec,
            lyric_object::AccessType::Public,
            {});
        auto functionUrl = declareFunctionResult.getResult();
        auto *call = cast_symbol_to_call(symbolCache->getOrImportSymbol(functionUrl).orElseThrow());
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<uint32_t>(StdTimeTrap::NOW));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareFunctionResult = block->declareFunction("ParseTimezone",
            {
                { {}, "name", "", StringSpec, lyric_parser::BindingType::VALUE },
            },
            {},
            {},
            TimezoneSpec,
            lyric_object::AccessType::Public,
            {});
        auto functionUrl = declareFunctionResult.getResult();
        auto *call = cast_symbol_to_call(symbolCache->getOrImportSymbol(functionUrl).orElseThrow());
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<uint32_t>(StdTimeTrap::PARSE_TIMEZONE));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return tempo_utils::Status();
}