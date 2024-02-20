/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <zuri_std_system/lib_types.h>

#include "compile_queue.h"

tempo_utils::Status
build_std_system_Queue(
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block)
{
    auto *symbolCache = state.symbolCache();

    auto resolveObjectResult = block->resolveClass(
        lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"Object"})));
    if (resolveObjectResult.isStatus())
        return resolveObjectResult.getStatus();
    auto *ObjectClass = static_cast<lyric_assembler::ClassSymbol *>(
        symbolCache->getSymbol(resolveObjectResult.getResult()));

    lyric_object::TemplateParameter TParam;
    TParam.name = "T";
    TParam.index = 0;
    TParam.typeDef = {};
    TParam.bound = lyric_object::BoundType::None;
    TParam.variance = lyric_object::VarianceType::Invariant;

    auto declareQueueClassResult = block->declareClass("Queue",
        ObjectClass, lyric_object::AccessType::Public, {TParam});
    if (declareQueueClassResult.isStatus())
        return declareQueueClassResult.getStatus();
    auto *QueueClass = static_cast<lyric_assembler::ClassSymbol *>(
        symbolCache->getSymbol(declareQueueClassResult.getResult()));

    auto TSpec = lyric_parser::Assignable::forSingular({"T"});
    auto FutureTSpec = lyric_parser::Assignable::forSingular(
        lyric_common::SymbolPath({"Future"}), {TSpec});
    auto BoolSpec = lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"Bool"}));

    {
        auto declareCtorResult = QueueClass->declareCtor(
            {},
            {},
            {},
            lyric_object::AccessType::Public,
            static_cast<tu_uint32>(StdSystemTrap::QUEUE_ALLOC));
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareCtorResult.getResult()));
        auto *code = call->callProc()->procCode();
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareMethodResult = QueueClass->declareMethod("push",
            {
                { {}, "element", "", TSpec, lyric_parser::BindingType::VALUE },
            },
            {},
            {},
            BoolSpec,
            lyric_object::AccessType::Public);
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareMethodResult.getResult()));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<uint32_t>(StdSystemTrap::QUEUE_PUSH));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareMethodResult = QueueClass->declareMethod("pop",
            {},
            {},
            {},
            FutureTSpec,
            lyric_object::AccessType::Public);
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareMethodResult.getResult()));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<uint32_t>(StdSystemTrap::QUEUE_POP));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return lyric_assembler::AssemblerStatus::ok();
}
