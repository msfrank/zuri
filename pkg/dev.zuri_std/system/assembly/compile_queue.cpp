/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/pack_builder.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/template_handle.h>
#include <lyric_assembler/type_cache.h>
#include <zuri_std_system/lib_types.h>

#include "compile_queue.h"

tempo_utils::Status
build_std_system_Queue(
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block)
{
    auto *fundamentalCache = state.fundamentalCache();
    auto *symbolCache = state.symbolCache();
    auto *typeCache = state.typeCache();

    auto resolveObjectResult = block->resolveClass(
        lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"Object"})));
    if (resolveObjectResult.isStatus())
        return resolveObjectResult.getStatus();
    auto *ObjectClass = cast_symbol_to_class(
        symbolCache->getOrImportSymbol(resolveObjectResult.getResult()).orElseThrow());

    auto BoolType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool);

    lyric_object::TemplateParameter TParam;
    TParam.name = "T";
    TParam.index = 0;
    TParam.typeDef = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
    TParam.bound = lyric_object::BoundType::None;
    TParam.variance = lyric_object::VarianceType::Invariant;

    auto futureUrl = lyric_common::SymbolUrl::fromString("#Future");

    auto declareQueueClassResult = block->declareClass("Queue",
        ObjectClass, lyric_object::AccessType::Public, {TParam});
    if (declareQueueClassResult.isStatus())
        return declareQueueClassResult.getStatus();
    auto *QueueClass = cast_symbol_to_class(
        symbolCache->getOrImportSymbol(declareQueueClassResult.getResult()).orElseThrow());

    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, QueueClass->declareCtor(
            lyric_object::AccessType::Public, static_cast<tu_uint32>(StdSystemTrap::QUEUE_ALLOC)));
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall({}, lyric_common::TypeDef::noReturn()));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    auto *templateHandle = QueueClass->classTemplate();
    auto TType = templateHandle->getPlaceholder("T");

    lyric_assembler::TypeHandle *futureTHandle;
    TU_ASSIGN_OR_RETURN (futureTHandle, typeCache->declareParameterizedType(futureUrl, {TType}));
    auto FutureTType = futureTHandle->getTypeDef();

    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, QueueClass->declareMethod(
            "Push", lyric_object::AccessType::Public));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("element", "", TType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, BoolType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdSystemTrap::QUEUE_PUSH));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, QueueClass->declareMethod(
            "Pop", lyric_object::AccessType::Public));
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall({}, FutureTType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdSystemTrap::QUEUE_POP));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return {};
}
