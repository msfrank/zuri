/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/field_symbol.h>
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
    auto *symbolCache = state.symbolCache();

    auto resolveRecordResult = block->resolveStruct(
        lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"Record"})));
    if (resolveRecordResult.isStatus())
        return resolveRecordResult.getStatus();
    auto *RecordStruct = cast_symbol_to_struct(
        symbolCache->getSymbol(resolveRecordResult.getResult()));

    auto declareAttrStructResult = block->declareStruct("Attr",
        RecordStruct, lyric_object::AccessType::Public, lyric_object::DeriveType::Any);
    if (declareAttrStructResult.isStatus())
        return declareAttrStructResult.getStatus();
    auto *AttrStruct = cast_symbol_to_struct(
        symbolCache->getSymbol(declareAttrStructResult.getResult()));
    return AttrStruct;
}

tempo_utils::Status
build_std_system_Attr(
    lyric_assembler::StructSymbol *AttrStruct,
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block)
{
    auto *symbolCache = state.symbolCache();

    auto UrlSpec = lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"Url"}));
    auto IntSpec = lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"Int"}));
    auto ValueSpec = lyric_parser::Assignable::forUnion({
        lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"Intrinsic"})),
        lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"Element"})),
    });

    auto declareNsResult = AttrStruct->declareMember("ns", UrlSpec);
    if (declareNsResult.isStatus())
        return declareNsResult.getStatus();
    auto *NsField = cast_symbol_to_field(
        symbolCache->getSymbol(declareNsResult.getResult().symbolUrl));

    auto declareIdResult = AttrStruct->declareMember("id", IntSpec);
    if (declareIdResult.isStatus())
        return declareIdResult.getStatus();
    auto *IdField = cast_symbol_to_field(
        symbolCache->getSymbol(declareIdResult.getResult().symbolUrl));

    auto declareValueResult = AttrStruct->declareMember("value", ValueSpec);
    if (declareValueResult.isStatus())
        return declareValueResult.getStatus();
    auto *ValueField = cast_symbol_to_field(
        symbolCache->getSymbol(declareValueResult.getResult().symbolUrl));

    {
        auto *SuperStruct = AttrStruct->superStruct();
        auto superCtorUrl = SuperStruct->getCtor();
        if (!symbolCache->hasSymbol(superCtorUrl))
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "missing ctor for Record");
        auto *superCtorSym = symbolCache->getSymbol(superCtorUrl);
        if (superCtorSym->getSymbolType() != lyric_assembler::SymbolType::CALL)
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid ctor for Record");
        auto *superCtorCall = cast_symbol_to_call(superCtorSym);
        superCtorCall->touch();

        auto declareCtorResult = AttrStruct->declareCtor(
            {
                {{}, "ns", "ns", UrlSpec, lyric_parser::BindingType::VALUE},
                {{}, "id", "id", IntSpec, lyric_parser::BindingType::VALUE},
                {{}, "value", "", ValueSpec, lyric_parser::BindingType::VALUE},
            },
            {},
            lyric_object::AccessType::Public,
            static_cast<uint32_t>(StdSystemTrap::ATTR_ALLOC));
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareCtorResult.getResult()));
        auto *code = call->callProc()->procCode();
        // call the super constructor
        code->loadSynthetic(lyric_assembler::SyntheticType::THIS);
        code->callVirtual(superCtorCall->getAddress(), 0);
        // load the ns argument and store it in member
        code->loadArgument(lyric_assembler::ArgumentOffset(0));
        code->storeField(NsField->getAddress());
        // load the id argument and store it in member
        code->loadArgument(lyric_assembler::ArgumentOffset(1));
        code->storeField(IdField->getAddress());
        // load the value argument and store it in member
        code->loadArgument(lyric_assembler::ArgumentOffset(2));
        code->storeField(ValueField->getAddress());
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return lyric_assembler::AssemblerStatus::ok();
}