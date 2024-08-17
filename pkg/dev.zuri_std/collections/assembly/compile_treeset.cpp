/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/pack_builder.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/template_handle.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_typing/callsite_reifier.h>
#include <zuri_std_collections/lib_types.h>

#include "compile_treeset.h"

tempo_utils::Result<lyric_assembler::ClassSymbol *>
declare_std_collections_TreeSet(
    lyric_assembler::ObjectState &state,
    lyric_assembler::BlockHandle *block)
{
    auto *fundamentalCache = state.fundamentalCache();

    lyric_assembler::ClassSymbol *ObjectClass;
    TU_ASSIGN_OR_RETURN (ObjectClass, block->resolveClass(
        fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object)));

    lyric_object::TemplateParameter TParam;
    TParam.name = "T";
    TParam.index = 0;
    TParam.typeDef = state.fundamentalCache()->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
    TParam.bound = lyric_object::BoundType::None;
    TParam.variance = lyric_object::VarianceType::Invariant;

    return block->declareClass("TreeSet", ObjectClass, lyric_object::AccessType::Public, {TParam});
}

tempo_utils::Status
build_std_collections_TreeSet(
    lyric_assembler::ClassSymbol *TreeSetClass,
    lyric_assembler::ClassSymbol *TreeSetIteratorClass,
    lyric_assembler::ObjectState &state,
    lyric_assembler::BlockHandle *parentBlock,
    lyric_typing::TypeSystem *typeSystem)
{
    auto *fundamentalCache = state.fundamentalCache();
    auto *symbolCache = state.symbolCache();
    auto *typeCache = state.typeCache();

    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    auto BoolType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool);
    auto NilType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Nil);

    auto orderedUrl = fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Ordered);

    lyric_assembler::CallAddress compareAddress;
    {
        lyric_object::TemplateParameter TParam;
        TParam.name = "T";
        TParam.index = 0;
        TParam.typeDef = state.fundamentalCache()->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
        TParam.bound = lyric_object::BoundType::None;
        TParam.variance = lyric_object::VarianceType::Invariant;

        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, parentBlock->declareFunction(
            "TreeSet.$compare", lyric_object::AccessType::Public, {TParam}));
        auto *templateHandle = callSymbol->callTemplate();
        auto TType = templateHandle->getPlaceholder("T");
        lyric_assembler::TypeHandle *orderedTHandle;
        TU_ASSIGN_OR_RETURN (orderedTHandle, typeCache->declareParameterizedType(orderedUrl, {TType}));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("x1", "", TType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("x2", "", TType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendCtxParameter("ord", "", orderedTHandle->getTypeDef()));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, IntType));
        auto *block = procHandle->procBlock();

        // push ord receiver onto the stack
        lyric_assembler::DataReference ord;
        TU_ASSIGN_OR_RETURN (ord, block->resolveReference("ord"));
        TU_RETURN_IF_NOT_OK (block->load(ord));

        // push Ordered concept descriptor onto the stack
        lyric_assembler::AbstractSymbol *sym;
        TU_ASSIGN_OR_RETURN (sym, symbolCache->getOrImportSymbol(orderedUrl));
        auto *orderedSymbol = cast_symbol_to_concept(sym);
        lyric_assembler::CallableInvoker invoker;
        TU_RETURN_IF_NOT_OK (orderedSymbol->prepareAction("compare", ord.typeDef, invoker));

        auto ordTypeArguments = ord.typeDef.getConcreteArguments();
        std::vector<lyric_common::TypeDef> callsiteTypeArguments(ordTypeArguments.begin(), ordTypeArguments.end());
        lyric_typing::CallsiteReifier reifier(typeSystem);
        TU_RETURN_IF_NOT_OK (reifier.initialize(invoker, callsiteTypeArguments));

        // push x1 and x2 onto the stack
        lyric_assembler::DataReference x1;
        TU_ASSIGN_OR_RETURN (x1, block->resolveReference("x1"));
        TU_RETURN_IF_NOT_OK (block->load(x1));
        TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(x1.typeDef));

        lyric_assembler::DataReference x2;
        TU_ASSIGN_OR_RETURN (x2, block->resolveReference("x2"));
        TU_RETURN_IF_NOT_OK (block->load(x2));
        TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(x2.typeDef));

        // return result of ord.compare()
        TU_RETURN_IF_STATUS (invoker.invoke(block, reifier));

        compareAddress = callSymbol->getAddress();
    }

    auto *templateHandle = TreeSetClass->classTemplate();
    auto TType = templateHandle->getPlaceholder("T");

    lyric_assembler::TypeHandle *orderedTHandle;
    TU_ASSIGN_OR_RETURN (orderedTHandle, typeCache->declareParameterizedType(orderedUrl, {TType}));
    auto OrderedTType = orderedTHandle->getTypeDef();

    lyric_assembler::TypeHandle *iterableTHandle;
    TU_ASSIGN_OR_RETURN (iterableTHandle, typeCache->declareParameterizedType(
        fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Iterable), {TType}));
    auto IterableTType = iterableTHandle->getTypeDef();

    lyric_assembler::TypeHandle *iteratorTHandle;
    TU_ASSIGN_OR_RETURN (iteratorTHandle, typeCache->declareParameterizedType(
        fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Iterator), {TType}));
    auto IteratorTType = iteratorTHandle->getTypeDef();

    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, TreeSetClass->declareCtor(
            lyric_object::AccessType::Public, static_cast<tu_uint32>(StdCollectionsTrap::TREESET_ALLOC)));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendRestParameter("", TType));
        TU_RETURN_IF_NOT_OK (packBuilder.appendCtxParameter("ord", "", OrderedTType));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, lyric_common::TypeDef::noReturn()));
        auto *codeBuilder = procHandle->procCode();

        // push TreeSet.$compare call descriptor onto the stack
        TU_RETURN_IF_NOT_OK (codeBuilder->loadCall(compareAddress));

        codeBuilder->trap(static_cast<tu_uint32>(StdCollectionsTrap::TREESET_CTOR));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, TreeSetClass->declareMethod(
            "Size", lyric_object::AccessType::Public));
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall({}, IntType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdCollectionsTrap::TREESET_SIZE));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, TreeSetClass->declareMethod(
            "Contains", lyric_object::AccessType::Public));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("value", "", TType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, BoolType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdCollectionsTrap::TREESET_CONTAINS));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, TreeSetClass->declareMethod(
            "Add", lyric_object::AccessType::Public));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("value", "", TType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, BoolType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdCollectionsTrap::TREESET_ADD));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, TreeSetClass->declareMethod(
            "Remove", lyric_object::AccessType::Public));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("value", "", TType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, BoolType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdCollectionsTrap::TREESET_REMOVE));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, TreeSetClass->declareMethod(
            "Clear", lyric_object::AccessType::Public));
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall({}, NilType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdCollectionsTrap::TREESET_CLEAR));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    lyric_assembler::ImplHandle *treesetIterableImplHandle;
    TU_ASSIGN_OR_RETURN (treesetIterableImplHandle, TreeSetClass->declareImpl(IterableTType));

    {
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, treesetIterableImplHandle->defineExtension("Iterate", {}, IteratorTType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->loadClass(TreeSetIteratorClass->getAddress());
        codeBuilder->trap(static_cast<tu_uint32>(StdCollectionsTrap::TREESET_ITERATE));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return {};
}
