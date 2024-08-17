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

#include "compile_vector.h"

tempo_utils::Result<lyric_assembler::ClassSymbol *>
declare_std_collections_Vector(
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
    TParam.variance = lyric_object::VarianceType::Covariant;

    return block->declareClass("Vector", ObjectClass, lyric_object::AccessType::Public, {TParam});
}

tempo_utils::Status
build_std_collections_Vector(
    lyric_assembler::ClassSymbol *VectorClass,
    lyric_assembler::ClassSymbol *VectorIteratorClass,
    lyric_assembler::ObjectState &state,
    lyric_assembler::BlockHandle *parentBlock,
    lyric_typing::TypeSystem *typeSystem)
{
    auto *fundamentalCache = state.fundamentalCache();
    auto *typeCache = state.typeCache();

    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    auto NilType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Nil);

    auto *templateHandle = VectorClass->classTemplate();
    auto TType = templateHandle->getPlaceholder("T");

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
        TU_ASSIGN_OR_RETURN (callSymbol, VectorClass->declareCtor(
            lyric_object::AccessType::Public, static_cast<tu_uint32>(StdCollectionsTrap::VECTOR_ALLOC)));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendRestParameter("", TType));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, lyric_common::TypeDef::noReturn()));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdCollectionsTrap::VECTOR_CTOR));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, VectorClass->declareMethod(
            "Size", lyric_object::AccessType::Public));
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall({}, IntType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdCollectionsTrap::VECTOR_SIZE));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, VectorClass->declareMethod(
            "At", lyric_object::AccessType::Public));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("index", "", IntType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, TType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdCollectionsTrap::VECTOR_AT));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, VectorClass->declareMethod(
            "Insert", lyric_object::AccessType::Public));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("index", "", IntType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("value", "", TType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, NilType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdCollectionsTrap::VECTOR_INSERT));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, VectorClass->declareMethod(
            "Append", lyric_object::AccessType::Public));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("value", "", TType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, NilType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdCollectionsTrap::VECTOR_APPEND));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, VectorClass->declareMethod(
            "Replace", lyric_object::AccessType::Public));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("index", "", IntType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("value", "", TType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, TType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdCollectionsTrap::VECTOR_UPDATE));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, VectorClass->declareMethod(
            "Remove", lyric_object::AccessType::Public));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("index", "", IntType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, TType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdCollectionsTrap::VECTOR_REMOVE));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, VectorClass->declareMethod(
            "Clear", lyric_object::AccessType::Public));
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall({}, NilType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdCollectionsTrap::VECTOR_CLEAR));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    lyric_assembler::ImplHandle *vectorIterableImplHandle;
    TU_ASSIGN_OR_RETURN (vectorIterableImplHandle, VectorClass->declareImpl(IterableTType));

    {
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, vectorIterableImplHandle->defineExtension("Iterate", {}, IteratorTType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->loadClass(VectorIteratorClass->getAddress());
        codeBuilder->trap(static_cast<tu_uint32>(StdCollectionsTrap::VECTOR_ITERATE));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return {};
}
