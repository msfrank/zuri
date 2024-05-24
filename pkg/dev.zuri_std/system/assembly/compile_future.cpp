/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <zuri_std_system/lib_types.h>

#include "compile_future.h"

tempo_utils::Status
build_std_system_Future(
    lyric_compiler::ModuleEntry &moduleEntry,
    lyric_assembler::BlockHandle *block)
{
    auto *state = moduleEntry.getState();
    auto *fundamentalCache = state->fundamentalCache();
    auto *symbolCache = state->symbolCache();

    auto resolveObjectResult = block->resolveClass(
        lyric_parser::Assignable::forSingular({"Object"}));
    if (resolveObjectResult.isStatus())
        return resolveObjectResult.getStatus();
    auto *ObjectClass = cast_symbol_to_class(
        symbolCache->getOrImportSymbol(resolveObjectResult.getResult()).orElseThrow());

    lyric_object::TemplateParameter TParam;
    TParam.name = "T";
    TParam.index = 0;
    TParam.typeDef = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
    TParam.bound = lyric_object::BoundType::None;
    TParam.variance = lyric_object::VarianceType::Invariant;

    auto declareFutureClassResult = block->declareClass("Future",
        ObjectClass, lyric_object::AccessType::Public, {TParam}, lyric_object::DeriveType::Final);
    if (declareFutureClassResult.isStatus())
        return declareFutureClassResult.getStatus();
    auto *FutureClass = cast_symbol_to_class(
        symbolCache->getOrImportSymbol(declareFutureClassResult.getResult()).orElseThrow());

    auto TSpec = lyric_parser::Assignable::forSingular({"T"});
    auto FutureTSpec = lyric_parser::Assignable::forSingular(declareFutureClassResult.getResult(), {TSpec});
    auto StatusSpec = lyric_parser::Assignable::forSingular(
        fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Status));
    auto BoolSpec = lyric_parser::Assignable::forSingular(
        fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Bool));

    {
        auto declareCtorResult = FutureClass->declareCtor(
            {},
            {},
            {},
            lyric_object::AccessType::Public,
            static_cast<tu_uint32>(StdSystemTrap::FUTURE_ALLOC));
        auto *call = cast_symbol_to_call(symbolCache->getOrImportSymbol(declareCtorResult.getResult()).orElseThrow());
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<tu_uint32>(StdSystemTrap::FUTURE_CTOR));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareMethodResult = FutureClass->declareMethod("Complete",
            {
                {{}, "result", "", TSpec, lyric_parser::BindingType::VALUE}
            },
            {},
            {},
            BoolSpec,
            lyric_object::AccessType::Public);
        auto *call = cast_symbol_to_call(symbolCache->getOrImportSymbol(declareMethodResult.getResult()).orElseThrow());
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<tu_uint32>(StdSystemTrap::FUTURE_COMPLETE));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareMethodResult = FutureClass->declareMethod("Reject",
            {
                {{}, "status", "", StatusSpec, lyric_parser::BindingType::VALUE}
            },
            {},
            {},
            BoolSpec,
            lyric_object::AccessType::Public);
        auto *call = cast_symbol_to_call(symbolCache->getOrImportSymbol(declareMethodResult.getResult()).orElseThrow());
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<tu_uint32>(StdSystemTrap::FUTURE_REJECT));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareMethodResult = FutureClass->declareMethod("Cancel",
            {},
            {},
            {},
            BoolSpec,
            lyric_object::AccessType::Public);
        auto *call = cast_symbol_to_call(symbolCache->getOrImportSymbol(declareMethodResult.getResult()).orElseThrow());
        moduleEntry.compileBlock(R"(
            val cancelled: Cancelled = Cancelled{message = "cancelled"}
            this.Reject(cancelled)
            )", call->callProc()->procBlock());
    }


    return tempo_utils::Status();
}
