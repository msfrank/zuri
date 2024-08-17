/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/pack_builder.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_typing/callsite_reifier.h>
#include <zuri_std_system/lib_types.h>

#include "compile_system.h"

tempo_utils::Status
build_std_system(
    lyric_assembler::ObjectState &state,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem)
{
    auto *fundamentalCache = state.fundamentalCache();
    auto *typeCache = state.typeCache();

    auto PortType = lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl::fromString("#Port"));
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    auto StatusType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Status);
    auto UndefType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Undef);
    auto UrlType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Url);

    auto futureUrl = lyric_common::SymbolUrl::fromString("#Future");

    lyric_assembler::TypeHandle *futureOfUndefHandle;
    TU_ASSIGN_OR_RETURN (futureOfUndefHandle, typeCache->declareParameterizedType(futureUrl, {UndefType}));
    auto FutureOfUndefType = futureOfUndefHandle->getTypeDef();

    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, block->declareFunction(
            "Acquire", lyric_object::AccessType::Public, {}));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("url", "", UrlType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, PortType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdSystemTrap::ACQUIRE));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_object::TemplateParameter TParam;
        TParam.name = "T";
        TParam.index = 0;
        TParam.typeDef = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
        TParam.bound = lyric_object::BoundType::None;
        TParam.variance = lyric_object::VarianceType::Invariant;

        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, block->declareFunction(
            "Await", lyric_object::AccessType::Public, {TParam}));

        auto *templateHandle = callSymbol->callTemplate();
        auto TType = templateHandle->getPlaceholder("T");

        lyric_assembler::TypeHandle *futureTHandle;
        TU_ASSIGN_OR_RETURN (futureTHandle, typeCache->declareParameterizedType(futureUrl, {TType}));
        auto FutureTType = futureTHandle->getTypeDef();

        auto StatusOrTType = lyric_common::TypeDef::forUnion({StatusType, TType});

        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("fut", "", FutureTType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, StatusOrTType));
        auto *codeBuilder = procHandle->procCode();

        // after the await trap completes the current coro may have been suspended
        codeBuilder->trap(static_cast<tu_uint32>(StdSystemTrap::AWAIT));

        // if the coro was suspended it is resumed at this instruction. after the future_result trap
        // completes, the result will be on the top of the stack.
        codeBuilder->trap(static_cast<tu_uint32>(StdSystemTrap::GET_RESULT));

        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_object::TemplateParameter TParam;
        TParam.name = "T";
        TParam.index = 0;
        TParam.typeDef = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
        TParam.bound = lyric_object::BoundType::None;
        TParam.variance = lyric_object::VarianceType::Invariant;

        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, block->declareFunction(
            "AwaitOrDefault", lyric_object::AccessType::Public, {TParam}));

        auto *templateHandle = callSymbol->callTemplate();
        auto TType = templateHandle->getPlaceholder("T");

        lyric_assembler::TypeHandle *futureTHandle;
        TU_ASSIGN_OR_RETURN (futureTHandle, typeCache->declareParameterizedType(futureUrl, {TType}));
        auto FutureTType = futureTHandle->getTypeDef();

        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("fut", "", FutureTType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("default", "", TType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, TType));
        auto *codeBuilder = procHandle->procCode();

        // after the await trap completes the current coro may have been suspended
        TU_RETURN_IF_NOT_OK (codeBuilder->trap(static_cast<tu_uint32>(StdSystemTrap::AWAIT)));

        // if the coro was suspended it is resumed at this instruction. after the future_result trap
        // completes, the result will be on the top of the stack.
        TU_RETURN_IF_NOT_OK (codeBuilder->trap(static_cast<tu_uint32>(StdSystemTrap::GET_RESULT)));

        // allocate a local and store the result in it
        auto result = procHandle->allocateLocal();
        TU_RETURN_IF_NOT_OK (codeBuilder->storeLocal(result));

        // load the result onto and push its type onto the top of the stack
        TU_RETURN_IF_NOT_OK (codeBuilder->loadLocal(result));
        TU_RETURN_IF_NOT_OK (codeBuilder->writeOpcode(lyric_object::Opcode::OP_TYPE_OF));

        // get the type handle for Status and push the Status type onto the top of the stack
        auto statusType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Status);
        lyric_assembler::TypeHandle *statusTypeHandle;
        TU_ASSIGN_OR_RETURN (statusTypeHandle, typeCache->getOrMakeType(statusType));
        TU_ASSERT (statusTypeHandle != nullptr);
        statusTypeHandle->touch();
        TU_RETURN_IF_NOT_OK (codeBuilder->loadType(statusTypeHandle->getAddress()));

        // perform type comparison result isA Status
        TU_RETURN_IF_NOT_OK (codeBuilder->writeOpcode(lyric_object::Opcode::OP_TYPE_CMP));

        // if lhs type equals or extends rhs, then jump to defaultLabel
        lyric_assembler::PatchOffset predicateJump;
        TU_ASSIGN_OR_RETURN (predicateJump, codeBuilder->jumpIfLessOrEqual());

        // if we did not jump then result was not status, so return result
        TU_RETURN_IF_NOT_OK (codeBuilder->loadLocal(result));
        TU_RETURN_IF_NOT_OK (codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN));

        // patch predicate jump to default label
        lyric_assembler::JumpLabel defaultLabel;
        TU_ASSIGN_OR_RETURN (defaultLabel, codeBuilder->makeLabel());
        TU_RETURN_IF_NOT_OK (codeBuilder->patch(predicateJump, defaultLabel));

        // result is a status, so return the default argument
        TU_RETURN_IF_NOT_OK (codeBuilder->loadArgument(lyric_assembler::ArgumentOffset(1)));
        TU_RETURN_IF_NOT_OK (codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN));
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, block->declareFunction(
            "Sleep", lyric_object::AccessType::Public, {}));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("millis", "", IntType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, FutureOfUndefType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdSystemTrap::SLEEP));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_object::TemplateParameter TParam;
        TParam.name = "T";
        TParam.index = 0;
        TParam.typeDef = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
        TParam.bound = lyric_object::BoundType::None;
        TParam.variance = lyric_object::VarianceType::Invariant;

        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, block->declareFunction(
            "Spawn", lyric_object::AccessType::Public, {TParam}));

        auto *templateHandle = callSymbol->callTemplate();
        auto TType = templateHandle->getPlaceholder("T");

        lyric_assembler::TypeHandle *function0ReturningTHandle;
        TU_ASSIGN_OR_RETURN (function0ReturningTHandle, typeCache->declareParameterizedType(
            fundamentalCache->getFunctionUrl(0), {TType}));
        auto Function0ReturningTType = function0ReturningTHandle->getTypeDef();

        lyric_assembler::TypeHandle *futureTHandle;
        TU_ASSIGN_OR_RETURN (futureTHandle, typeCache->declareParameterizedType(futureUrl, {TType}));
        auto FutureTType = futureTHandle->getTypeDef();

        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("fun", "", Function0ReturningTType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, FutureTType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdSystemTrap::SPAWN));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return {};
}