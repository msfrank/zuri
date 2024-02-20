/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <zuri_std_collections/lib_types.h>

#include "compile_option.h"

tempo_utils::Status
build_std_collections_Option(
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *parentBlock,
    lyric_typing::TypeSystem *typeSystem)
{
    auto *fundamentalCache = state.fundamentalCache();
    auto *symbolCache = state.symbolCache();

    auto resolveObjectResult = parentBlock->resolveClass(
        lyric_parser::Assignable::forSingular({"Object"}));
    if (resolveObjectResult.isStatus())
        return resolveObjectResult.getStatus();
    auto *ObjectClass = cast_symbol_to_class(symbolCache->getSymbol(resolveObjectResult.getResult()));

    lyric_object::TemplateParameter TParam;
    TParam.name = "T";
    TParam.index = 0;
    TParam.typeDef = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
    TParam.bound = lyric_object::BoundType::Extends;
    TParam.variance = lyric_object::VarianceType::Covariant;

    auto declareOptionClassResult = parentBlock->declareClass("Option",
       ObjectClass, lyric_object::AccessType::Public, {TParam});
    if (declareOptionClassResult.isStatus())
        return declareOptionClassResult.getStatus();
    auto *OptionClass = cast_symbol_to_class(symbolCache->getSymbol(declareOptionClassResult.getResult()));

    auto TSpec = lyric_parser::Assignable::forSingular({"T"});
    auto NilSpec = lyric_parser::Assignable::forSingular(
        fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Nil));
    auto NilOrTSpec = lyric_parser::Assignable::forUnion({NilSpec, TSpec});

    auto BoolSpec = lyric_parser::Assignable::forSingular(
        fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Bool));
    auto IntSpec = lyric_parser::Assignable::forSingular(
        fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Int));
    auto IteratorTSpec = lyric_parser::Assignable::forSingular(
        fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Iterator), {TSpec});

    {
        auto declareCtorResult = OptionClass->declareCtor(
            {
                {{}, "value", "", NilOrTSpec, lyric_parser::BindingType::VALUE},
            },
            {},
            {},
            lyric_object::AccessType::Public,
            static_cast<tu_uint32>(StdCollectionsTrap::OPTION_ALLOC));
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareCtorResult.getResult()));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<tu_uint32>(StdCollectionsTrap::OPTION_CTOR));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareMethodResult = OptionClass->declareMethod("IsEmpty",
            {},
            {},
            {},
            BoolSpec,
            lyric_object::AccessType::Public);
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareMethodResult.getResult()));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<tu_uint32>(StdCollectionsTrap::OPTION_IS_EMPTY));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareMethodResult = OptionClass->declareMethod("Get",
            {},
            {},
            {},
            TSpec,
            lyric_object::AccessType::Public);
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareMethodResult.getResult()));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<tu_uint32>(StdCollectionsTrap::OPTION_GET));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return lyric_assembler::AssemblerStatus::ok();
}
