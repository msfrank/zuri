/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/pack_builder.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/template_handle.h>
#include <lyric_assembler/type_cache.h>
#include <zuri_std_collections/lib_types.h>

#include "compile_treeset_iterator.h"

tempo_utils::Result<lyric_assembler::ClassSymbol *>
declare_std_collections_TreeSetIterator(
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block)
{
    auto *fundamentalCache = state.fundamentalCache();
    auto *symbolCache = state.symbolCache();

    lyric_assembler::ClassSymbol *ObjectClass;
    TU_ASSIGN_OR_RETURN (ObjectClass, block->resolveClass(
        fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object)));

    lyric_object::TemplateParameter TParam;
    TParam.name = "T";
    TParam.index = 0;
    TParam.typeDef = state.fundamentalCache()->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
    TParam.bound = lyric_object::BoundType::None;
    TParam.variance = lyric_object::VarianceType::Contravariant;

    auto declareTreeSetIteratorClassResult = block->declareClass("TreeSetIterator",
        ObjectClass, lyric_object::AccessType::Public, {TParam});
    if (declareTreeSetIteratorClassResult.isStatus())
        return declareTreeSetIteratorClassResult.getStatus();
    auto *TreeSetIteratorClass = cast_symbol_to_class(
        symbolCache->getOrImportSymbol(declareTreeSetIteratorClassResult.getResult()).orElseThrow());
    return TreeSetIteratorClass;
}

tempo_utils::Status
build_std_collections_TreeSetIterator(
    lyric_assembler::ClassSymbol *TreeSetIteratorClass,
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem)
{
    auto *fundamentalCache = state.fundamentalCache();
    auto *typeCache = state.typeCache();

    auto BoolType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool);

    auto *templateHandle = TreeSetIteratorClass->classTemplate();
    auto TType = templateHandle->getPlaceholder("T");

    lyric_assembler::TypeHandle *iteratorTHandle;
    TU_ASSIGN_OR_RETURN (iteratorTHandle, typeCache->declareParameterizedType(
        fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Iterator), {TType}));
    auto IteratorTType = iteratorTHandle->getTypeDef();

    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, TreeSetIteratorClass->declareCtor(
            lyric_object::AccessType::Public, static_cast<tu_uint32>(StdCollectionsTrap::TREESET_ITERATOR_ALLOC)));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendRestParameter("", TType));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, lyric_common::TypeDef::noReturn()));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    lyric_assembler::ImplHandle *iteratorImplHandle;
    TU_ASSIGN_OR_RETURN (iteratorImplHandle, TreeSetIteratorClass->declareImpl(IteratorTType));

    {
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, iteratorImplHandle->defineExtension("Valid", {}, BoolType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdCollectionsTrap::TREESET_ITERATOR_VALID));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, iteratorImplHandle->defineExtension("Next", {}, TType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdCollectionsTrap::TREESET_ITERATOR_NEXT));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return {};
}
