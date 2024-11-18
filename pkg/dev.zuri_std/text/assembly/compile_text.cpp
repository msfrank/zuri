/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/pack_builder.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <zuri_std_text/lib_types.h>

#include "compile_text.h"

tempo_utils::Status
build_std_text_Text(
    lyric_assembler::ObjectState &state,
    lyric_assembler::NamespaceSymbol *globalNamespace)
{
    auto *block = globalNamespace->namespaceBlock();
    auto *fundamentalCache = state.fundamentalCache();

    lyric_assembler::ClassSymbol *ObjectClass;
    TU_ASSIGN_OR_RETURN (ObjectClass, block->resolveClass(
        fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object)));

    lyric_assembler::ClassSymbol *TextClass;
    TU_ASSIGN_OR_RETURN (TextClass, block->declareClass(
        "Text", ObjectClass, lyric_object::AccessType::Public, {}));

    TU_RETURN_IF_NOT_OK (globalNamespace->putBinding(
        TextClass->getSymbolUrl().getSymbolName(), TextClass->getSymbolUrl(), TextClass->getAccessType()));

    auto StringType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::String);
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    auto CharType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Char);

    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, TextClass->declareCtor(
            lyric_object::AccessType::Public, static_cast<tu_uint32>(StdTextTrap::TEXT_ALLOC)));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("string", "", StringType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, lyric_common::TypeDef::noReturn()));
        auto *procBuilder = procHandle->procCode();
        auto *fragment = procBuilder->rootFragment();
        fragment->trap(static_cast<tu_uint32>(StdTextTrap::TEXT_CTOR), 0);
        fragment->returnToCaller();
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, TextClass->declareMethod("Length", lyric_object::AccessType::Public));
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall({}, IntType));
        auto *procBuilder = procHandle->procCode();
        auto *fragment = procBuilder->rootFragment();
        fragment->trap(static_cast<tu_uint32>(StdTextTrap::TEXT_LENGTH), 0);
        fragment->returnToCaller();
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, TextClass->declareMethod("At", lyric_object::AccessType::Public));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("index", "", IntType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, CharType));
        auto *procBuilder = procHandle->procCode();
        auto *fragment = procBuilder->rootFragment();
        fragment->trap(static_cast<tu_uint32>(StdTextTrap::TEXT_AT), 0);
        fragment->returnToCaller();
    }

    return {};
}
