/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/pack_builder.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <zuri_std_system/lib_types.h>

#include "compile_attr.h"

tempo_utils::Result<lyric_assembler::StructSymbol *>
declare_std_system_Attr(
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block)
{
    auto *fundamentalCache = state.fundamentalCache();
    auto *symbolCache = state.symbolCache();

    lyric_assembler::StructSymbol *RecordStruct;
    TU_ASSIGN_OR_RETURN (RecordStruct, block->resolveStruct(
        fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Record)));

    auto declareAttrStructResult = block->declareStruct("Attr",
        RecordStruct, lyric_object::AccessType::Public, lyric_object::DeriveType::Any);
    if (declareAttrStructResult.isStatus())
        return declareAttrStructResult.getStatus();
    auto *AttrStruct = cast_symbol_to_struct(
        symbolCache->getOrImportSymbol(declareAttrStructResult.getResult()).orElseThrow());
    return AttrStruct;
}

tempo_utils::Status
build_std_system_Attr(
    lyric_assembler::StructSymbol *AttrStruct,
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block)
{
    auto *fundamentalCache = state.fundamentalCache();
    auto *symbolCache = state.symbolCache();

    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    auto UrlType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Url);
    auto IntrinsicType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Intrinsic);
    auto UndefType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Undef);
    auto ElementType = lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl::fromString("#Element"));
    auto ValueType = lyric_common::TypeDef::forUnion({ IntrinsicType, ElementType });

    auto declareNsResult = AttrStruct->declareMember("ns", UrlType);
    if (declareNsResult.isStatus())
        return declareNsResult.getStatus();
    auto *NsField = cast_symbol_to_field(
        symbolCache->getOrImportSymbol(declareNsResult.getResult().symbolUrl).orElseThrow());

    auto declareIdResult = AttrStruct->declareMember("id", IntType);
    if (declareIdResult.isStatus())
        return declareIdResult.getStatus();
    auto *IdField = cast_symbol_to_field(
        symbolCache->getOrImportSymbol(declareIdResult.getResult().symbolUrl).orElseThrow());

    auto declareValueResult = AttrStruct->declareMember("value", ValueType);
    if (declareValueResult.isStatus())
        return declareValueResult.getStatus();
    auto *ValueField = cast_symbol_to_field(
        symbolCache->getOrImportSymbol(declareValueResult.getResult().symbolUrl).orElseThrow());

    lyric_common::SymbolUrl valueInitializerUrl;
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, block->declareFunction(
            "Attr.$init$value", lyric_object::AccessType::Public, {}));
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall( {}, UndefType));
        auto *codeBuilder = procHandle->procCode();
        TU_RETURN_IF_NOT_OK (codeBuilder->loadUndef());
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
        valueInitializerUrl = callSymbol->getSymbolUrl();
    }

    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, AttrStruct->declareCtor(
            lyric_object::AccessType::Public, static_cast<tu_uint32>(StdSystemTrap::ATTR_ALLOC)));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("ns", "", UrlType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("id", "", IntType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendListOptParameter("value", "", ValueType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, lyric_common::TypeDef::noReturn()));
        callSymbol->putInitializer("value", valueInitializerUrl);
        auto *codeBuilder = procHandle->procCode();

        auto *SuperStruct = AttrStruct->superStruct();
        auto superCtorUrl = SuperStruct->getCtor();
        if (!symbolCache->hasSymbol(superCtorUrl))
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "missing ctor for Record");
        auto *superCtorSym = symbolCache->getOrImportSymbol(superCtorUrl).orElseThrow();
        if (superCtorSym->getSymbolType() != lyric_assembler::SymbolType::CALL)
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid ctor for Record");
        auto *superCtorCall = cast_symbol_to_call(superCtorSym);
        superCtorCall->touch();

        // call the super constructor
        codeBuilder->loadSynthetic(lyric_assembler::SyntheticType::THIS);
        codeBuilder->callVirtual(superCtorCall->getAddress(), 0);
        // load the ns argument and store it in member
        codeBuilder->loadArgument(lyric_assembler::ArgumentOffset(0));
        codeBuilder->storeField(NsField->getAddress());
        // load the id argument and store it in member
        codeBuilder->loadArgument(lyric_assembler::ArgumentOffset(1));
        codeBuilder->storeField(IdField->getAddress());
        // load the value argument and store it in member
        codeBuilder->loadArgument(lyric_assembler::ArgumentOffset(2));
        codeBuilder->storeField(ValueField->getAddress());
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return {};
}