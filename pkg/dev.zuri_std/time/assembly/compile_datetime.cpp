/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <zuri_std_time/lib_types.h>

#include "compile_datetime.h"

tempo_utils::Status
build_std_time_Datetime(
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block)
{
    auto *symbolCache = state.symbolCache();

    auto resolveObjectResult = block->resolveClass(
        lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"Object"})));
    if (resolveObjectResult.isStatus())
        return resolveObjectResult.getStatus();
    auto *ObjectClass = cast_symbol_to_class(
        symbolCache->getOrImportSymbol(resolveObjectResult.getResult()).orElseThrow());

    auto declareDatetimeClassResult = block->declareClass(
        "Datetime", ObjectClass, lyric_object::AccessType::Public, {});
    if (declareDatetimeClassResult.isStatus())
        return declareDatetimeClassResult.getStatus();
    auto *DatetimeClass = cast_symbol_to_class(
        symbolCache->getOrImportSymbol(declareDatetimeClassResult.getResult()).orElseThrow());

    auto StringSpec = lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"String"}));
    auto IntSpec = lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"Int"}));
    auto InstantSpec = lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"Instant"}));
    auto TimezoneSpec = lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"Timezone"}));

    {
        auto declareCtorResult = DatetimeClass->declareCtor(
            {
                { {}, "instant", "", InstantSpec, lyric_parser::BindingType::VALUE },
                { {}, "timezone", "", TimezoneSpec, lyric_parser::BindingType::VALUE },
            },
            {},
            {},
            lyric_object::AccessType::Public,
            static_cast<tu_uint32>(StdTimeTrap::DATETIME_ALLOC));
        auto *call = cast_symbol_to_call(
            symbolCache->getOrImportSymbol(declareCtorResult.getResult()).orElseThrow());
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<tu_uint32>(StdTimeTrap::DATETIME_CTOR));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return tempo_utils::Status();
}
