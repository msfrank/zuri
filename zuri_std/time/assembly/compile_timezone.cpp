/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <zuri_std_time/lib_types.h>

#include "compile_timezone.h"

tempo_utils::Status
build_std_time_Timezone(
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block)
{
    auto *symbolCache = state.symbolCache();

    auto resolveObjectResult = block->resolveClass(
        lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"Object"})));
    if (resolveObjectResult.isStatus())
        return resolveObjectResult.getStatus();
    auto *ObjectClass = cast_symbol_to_class(symbolCache->getSymbol(resolveObjectResult.getResult()));

    auto declareTimezoneClassResult = block->declareClass(
        "Timezone", ObjectClass, lyric_object::AccessType::Public, {});
    if (declareTimezoneClassResult.isStatus())
        return declareTimezoneClassResult.getStatus();
    auto *TimezoneClass = cast_symbol_to_class(symbolCache->getSymbol(declareTimezoneClassResult.getResult()));

    auto StringSpec = lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"String"}));
    auto IntSpec = lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"Int"}));

    {
        auto declareCtorResult = TimezoneClass->declareCtor(
            {
                {{}, "offset", "", IntSpec, lyric_parser::BindingType::VALUE},
            },
            {},
            {},
            lyric_object::AccessType::Public,
            static_cast<tu_uint32>(StdTimeTrap::TIMEZONE_ALLOC));
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareCtorResult.getResult()));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<tu_uint32>(StdTimeTrap::TIMEZONE_CTOR));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return tempo_utils::Status();
}
