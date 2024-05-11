/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <zuri_std_time/lib_types.h>

#include "compile_instant.h"

tempo_utils::Status
build_std_time_Instant(
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block)
{
    auto *symbolCache = state.symbolCache();

    auto resolveObjectResult = block->resolveClass(
        lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"Object"})));
    if (resolveObjectResult.isStatus())
        return resolveObjectResult.getStatus();
    auto *ObjectClass = cast_symbol_to_class(symbolCache->getSymbol(resolveObjectResult.getResult()));

    auto declareInstantClassResult = block->declareClass(
        "Instant", ObjectClass, lyric_object::AccessType::Public, {});
    if (declareInstantClassResult.isStatus())
        return declareInstantClassResult.getStatus();
    auto *InstantClass = cast_symbol_to_class(symbolCache->getSymbol(declareInstantClassResult.getResult()));

    auto StringSpec = lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"String"}));
    auto IntSpec = lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"Int"}));

    {
        auto declareCtorResult = InstantClass->declareCtor(
            {},
            {},
            {},
            lyric_object::AccessType::Public,
            static_cast<tu_uint32>(StdTimeTrap::INSTANT_ALLOC));
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareCtorResult.getResult()));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<tu_uint32>(StdTimeTrap::INSTANT_CTOR));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareMethodResult = InstantClass->declareMethod("ToEpochMillis",
            {},
            {},
            {},
            IntSpec,
            lyric_object::AccessType::Public);
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareMethodResult.getResult()));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<tu_uint32>(StdTimeTrap::INSTANT_TO_EPOCH_MILLIS));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return tempo_utils::Status();
}
