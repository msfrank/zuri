/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <zuri_std_text/lib_types.h>

#include "compile_text.h"

tempo_utils::Status
build_std_text_Text(
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block)
{
    auto *symbolCache = state.symbolCache();

    auto resolveObjectResult = block->resolveClass(
        lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"Object"})));
    if (resolveObjectResult.isStatus())
        return resolveObjectResult.getStatus();
    auto *ObjectClass = cast_symbol_to_class(symbolCache->getSymbol(resolveObjectResult.getResult()));

    auto declareTextClassResult = block->declareClass(
        "Text", ObjectClass, lyric_object::AccessType::Public, {});
    if (declareTextClassResult.isStatus())
        return declareTextClassResult.getStatus();
    auto *TextClass = cast_symbol_to_class(symbolCache->getSymbol(declareTextClassResult.getResult()));

    auto StringSpec = lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"String"}));
    auto IntSpec = lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"Int"}));
    auto CharSpec = lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"Char"}));

    {
        auto declareCtorResult = TextClass->declareCtor(
            {
                {{}, "string", "", StringSpec, lyric_parser::BindingType::VALUE, {}}
            },
            {},
            {},
            lyric_object::AccessType::Public,
            static_cast<tu_uint32>(StdTextTrap::TEXT_ALLOC));
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareCtorResult.getResult()));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<tu_uint32>(StdTextTrap::TEXT_CTOR));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareMethodResult = TextClass->declareMethod("Length",
            {},
            {},
            {},
            IntSpec,
            lyric_object::AccessType::Public);
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareMethodResult.getResult()));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<tu_uint32>(StdTextTrap::TEXT_LENGTH));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareMethodResult = TextClass->declareMethod("At",
            {
                {{}, "index", "", IntSpec, lyric_parser::BindingType::VALUE, {}}
            },
            {},
            {},
            CharSpec,
            lyric_object::AccessType::Public);
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareMethodResult.getResult()));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<tu_uint32>(StdTextTrap::TEXT_AT));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return tempo_utils::Status();
}
