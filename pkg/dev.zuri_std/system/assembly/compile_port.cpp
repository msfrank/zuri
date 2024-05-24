/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <zuri_std_system/lib_types.h>

#include "compile_port.h"

tempo_utils::Status
build_std_system_Port(
    lyric_assembler::StructSymbol *OperationStruct,
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

    auto declarePortClassResult = block->declareClass("Port",
        ObjectClass,
        lyric_object::AccessType::Public,
        {},
        lyric_object::DeriveType::Sealed,
        true);
    if (declarePortClassResult.isStatus())
        return declarePortClassResult.getStatus();
    auto *PortClass = cast_symbol_to_class(
        symbolCache->getOrImportSymbol(declarePortClassResult.getResult()).orElseThrow());

    auto OperationSpec = lyric_parser::Assignable::fromTypeDef(OperationStruct->getAssignableType());
    auto FutureOfOperationSpec = lyric_parser::Assignable::forSingular(
        lyric_common::SymbolPath({"Future"}), {OperationSpec});
    auto BoolSpec = lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"Bool"}));

    {
        auto declareCtorResult = PortClass->declareCtor(
            {},
            {},
            {},
            lyric_object::AccessType::Private);
        auto *call = cast_symbol_to_call(symbolCache->getOrImportSymbol(declareCtorResult.getResult()).orElseThrow());
        auto *code = call->callProc()->procCode();
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareMethodResult = PortClass->declareMethod("Send",
            {
                { {}, "out", "", OperationSpec, lyric_parser::BindingType::VALUE },
            },
            {},
            {},
            BoolSpec,
            lyric_object::AccessType::Public);
        auto *call = cast_symbol_to_call(symbolCache->getOrImportSymbol(declareMethodResult.getResult()).orElseThrow());
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<uint32_t>(StdSystemTrap::PORT_SEND));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareMethodResult = PortClass->declareMethod("Receive",
            {},
            {},
            {},
            FutureOfOperationSpec,
            lyric_object::AccessType::Public);
        auto *call = cast_symbol_to_call(symbolCache->getOrImportSymbol(declareMethodResult.getResult()).orElseThrow());
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<uint32_t>(StdSystemTrap::PORT_RECEIVE));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return lyric_assembler::AssemblerStatus::ok();
}
