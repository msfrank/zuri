/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/pack_builder.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_typing/callsite_reifier.h>
#include <zuri_std_system/lib_types.h>

#include "compile_element.h"

tempo_utils::Result<lyric_assembler::StructSymbol *>
declare_std_system_Element(
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block)
{
    auto *fundamentalCache = state.fundamentalCache();
    auto *symbolCache = state.symbolCache();

    lyric_assembler::StructSymbol *RecordStruct;
    TU_ASSIGN_OR_RETURN (RecordStruct, block->resolveStruct(
        fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Record)));

    auto declareElementStructResult = block->declareStruct("Element",
        RecordStruct, lyric_object::AccessType::Public, lyric_object::DeriveType::Any);
    if (declareElementStructResult.isStatus())
        return declareElementStructResult.getStatus();
    auto *ElementStruct = cast_symbol_to_struct(
        symbolCache->getOrImportSymbol(declareElementStructResult.getResult()).orElseThrow());
    return ElementStruct;
}

tempo_utils::Status
build_std_system_Element(
    lyric_assembler::StructSymbol *ElementStruct,
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem)
{
    auto *fundamentalCache = state.fundamentalCache();
    auto *symbolCache = state.symbolCache();

    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    auto SeqType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Seq);
    auto UrlType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Url);
    auto IntrinsicType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Intrinsic);
    auto AttrType = lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl::fromString("#Attr"));
    auto ElementType = lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl::fromString("#Element"));
    auto ValueType = lyric_common::TypeDef::forUnion({ IntrinsicType, AttrType, ElementType });

    auto declareNsResult = ElementStruct->declareMember("ns", UrlType);
    if (declareNsResult.isStatus())
        return declareNsResult.getStatus();
    auto *NsField = cast_symbol_to_field(
        symbolCache->getOrImportSymbol(declareNsResult.getResult().symbolUrl).orElseThrow());

    auto declareIdResult = ElementStruct->declareMember("id", IntType);
    if (declareIdResult.isStatus())
        return declareIdResult.getStatus();
    auto *IdField = cast_symbol_to_field(
        symbolCache->getOrImportSymbol(declareIdResult.getResult().symbolUrl).orElseThrow());

    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, ElementStruct->declareCtor(
            lyric_object::AccessType::Public, static_cast<tu_uint32>(StdSystemTrap::ELEMENT_ALLOC)));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("ns", "", UrlType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("id", "", IntType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendRestParameter("", ValueType));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, lyric_common::TypeDef::noReturn()));
        auto *codeBuilder = procHandle->procCode();

        auto *SuperStruct = ElementStruct->superStruct();
        auto superCtorUrl = SuperStruct->getCtor();
        if (!symbolCache->hasSymbol(superCtorUrl))
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "missing ctor for Element");
        auto *superCtorSym = symbolCache->getOrImportSymbol(superCtorUrl).orElseThrow();
        if (superCtorSym->getSymbolType() != lyric_assembler::SymbolType::CALL)
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid ctor for Element");
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

        // invoke the Element constructor
        codeBuilder->trap(static_cast<tu_uint32>(StdSystemTrap::ELEMENT_CTOR));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, ElementStruct->declareMethod(
            "Size", lyric_object::AccessType::Public));
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall({}, IntType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdSystemTrap::ELEMENT_SIZE));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, ElementStruct->declareMethod(
            "GetOrElse", lyric_object::AccessType::Public));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("index", "", IntType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("default", "", ValueType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, ValueType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdSystemTrap::ELEMENT_GET_OR_ELSE));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return {};
}