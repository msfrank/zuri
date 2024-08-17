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

#include "compile_future.h"

tempo_utils::Status
build_std_system_Future(
    lyric_compiler::ModuleEntry &moduleEntry,
    lyric_assembler::BlockHandle *block)
{
    auto *state = moduleEntry.getState();
    auto *fundamentalCache = state->fundamentalCache();
    auto *typeCache = state->typeCache();

    lyric_assembler::ClassSymbol *ObjectClass;
    TU_ASSIGN_OR_RETURN (ObjectClass, block->resolveClass(
        fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object)));

    auto BoolType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool);
    auto StatusType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Status);

    lyric_object::TemplateParameter TParam;
    TParam.name = "T";
    TParam.index = 0;
    TParam.typeDef = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
    TParam.bound = lyric_object::BoundType::None;
    TParam.variance = lyric_object::VarianceType::Invariant;

    lyric_assembler::ClassSymbol *FutureClass;
    TU_ASSIGN_OR_RETURN (FutureClass, block->declareClass(
        "Future", ObjectClass, lyric_object::AccessType::Public, {TParam}, lyric_object::DeriveType::Final));

    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, FutureClass->declareCtor(
            lyric_object::AccessType::Public, static_cast<tu_uint32>(StdSystemTrap::FUTURE_ALLOC)));
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall({}, lyric_common::TypeDef::noReturn()));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdSystemTrap::FUTURE_CTOR));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    auto *templateHandle = FutureClass->classTemplate();
    auto TType = templateHandle->getPlaceholder("T");
    auto TOrStatusType = lyric_common::TypeDef::forUnion({TType, StatusType});

    lyric_assembler::TypeHandle *futureTHandle;
    TU_ASSIGN_OR_RETURN (futureTHandle, typeCache->declareParameterizedType(
        FutureClass->getSymbolUrl(), {TType}));
    auto FutureTType = futureTHandle->getTypeDef();

    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, FutureClass->declareMethod(
            "Complete", lyric_object::AccessType::Public));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("result", "", TType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, BoolType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdSystemTrap::FUTURE_COMPLETE));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, FutureClass->declareMethod(
            "Reject", lyric_object::AccessType::Public));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("status", "", StatusType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, BoolType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdSystemTrap::FUTURE_REJECT));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, FutureClass->declareMethod(
            "Cancel", lyric_object::AccessType::Public));
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall({}, BoolType));
        TU_RETURN_IF_STATUS (moduleEntry.compileBlock(R"(
            val cancelled: Cancelled = Cancelled{message = "cancelled"}
            this.Reject(cancelled)
            )", procHandle->procBlock()));
    }
    {
        lyric_object::TemplateParameter UParam;
        UParam.name = "U";
        UParam.index = 0;
        UParam.typeDef = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
        UParam.bound = lyric_object::BoundType::None;
        UParam.variance = lyric_object::VarianceType::Invariant;

        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, FutureClass->declareMethod(
            "Then", lyric_object::AccessType::Public, {UParam}));

        auto *functionTemplateHandle = callSymbol->callTemplate();
        auto UType = functionTemplateHandle->getPlaceholder("U");
        auto UOrStatusType = lyric_common::TypeDef::forUnion({UType, StatusType});

        lyric_assembler::TypeHandle *thenFunctionHandle;
        TU_ASSIGN_OR_RETURN (thenFunctionHandle, typeCache->declareFunctionType(
            UOrStatusType, {TOrStatusType}, {}));
        auto ThenFunctionType = thenFunctionHandle->getTypeDef();

        lyric_assembler::TypeHandle *futureUHandle;
        TU_ASSIGN_OR_RETURN (futureUHandle, typeCache->declareParameterizedType(
            FutureClass->getSymbolUrl(), {UType}));
        auto FutureUType = futureUHandle->getTypeDef();

        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("f", "", ThenFunctionType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, FutureUType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdSystemTrap::FUTURE_THEN));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return {};
}
