/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <zuri_std_collections/lib_types.h>

#include "compile_vector.h"

tempo_utils::Result<lyric_assembler::ClassSymbol *>
declare_std_collections_Vector(
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block)
{
    auto *symbolCache = state.symbolCache();

    auto resolveObjectResult = block->resolveClass(
        lyric_parser::Assignable::forSingular({"Object"}));
    if (resolveObjectResult.isStatus())
        return resolveObjectResult.getStatus();
    auto *ObjectClass = cast_symbol_to_class(symbolCache->getSymbol(resolveObjectResult.getResult()));

    lyric_object::TemplateParameter TParam;
    TParam.name = "T";
    TParam.index = 0;
    TParam.typeDef = state.fundamentalCache()->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
    TParam.bound = lyric_object::BoundType::None;
    TParam.variance = lyric_object::VarianceType::Covariant;

    auto declareVectorClassResult = block->declareClass("Vector",
        ObjectClass, lyric_object::AccessType::Public, {TParam});
    if (declareVectorClassResult.isStatus())
        return declareVectorClassResult.getStatus();
    auto *VectorClass = cast_symbol_to_class(symbolCache->getSymbol(declareVectorClassResult.getResult()));
    return VectorClass;
}

tempo_utils::Status
build_std_collections_Vector(
    lyric_assembler::ClassSymbol *VectorClass,
    lyric_assembler::ClassSymbol *VectorIteratorClass,
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *parentBlock,
    lyric_typing::TypeSystem *typeSystem)
{
    auto *symbolCache = state.symbolCache();

    auto TSpec = lyric_parser::Assignable::forSingular({"T"});
    auto IntSpec = lyric_parser::Assignable::forSingular({"Int"});
    auto NilSpec = lyric_parser::Assignable::forSingular({"Nil"});
    auto IterableTSpec = lyric_parser::Assignable::forSingular({"Iterable"}, {TSpec});
    auto IteratorTSpec = lyric_parser::Assignable::forSingular({"Iterator"}, {TSpec});

    {
        lyric_assembler::ParameterSpec restSpec;
        restSpec.type = TSpec;
        restSpec.binding = lyric_parser::BindingType::VALUE;
        restSpec.name = {};
        auto declareCtorResult = VectorClass->declareCtor(
            {},
            Option<lyric_assembler::ParameterSpec>(restSpec),
            {},
            lyric_object::AccessType::Public,
            static_cast<uint32_t>(StdCollectionsTrap::VECTOR_ALLOC));
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareCtorResult.getResult()));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<uint32_t>(StdCollectionsTrap::VECTOR_CTOR));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareMethodResult = VectorClass->declareMethod("size",
            {},
            {},
            {},
            IntSpec,
            lyric_object::AccessType::Public);
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareMethodResult.getResult()));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<uint32_t>(StdCollectionsTrap::VECTOR_SIZE));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareMethodResult = VectorClass->declareMethod("at",
            {
                {{}, "index", "", IntSpec, lyric_parser::BindingType::VALUE}
            },
            {},
            {},
            TSpec,
            lyric_object::AccessType::Public);
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareMethodResult.getResult()));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<uint32_t>(StdCollectionsTrap::VECTOR_AT));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareMethodResult = VectorClass->declareMethod("insert",
            {
                {{}, "index", "", IntSpec, lyric_parser::BindingType::VALUE},
                {{}, "value", "", TSpec, lyric_parser::BindingType::VALUE},
            },
            {},
            {},
            NilSpec,
            lyric_object::AccessType::Public);
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareMethodResult.getResult()));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<uint32_t>(StdCollectionsTrap::VECTOR_INSERT));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareMethodResult = VectorClass->declareMethod("append",
            {
                {{}, "value", "", TSpec, lyric_parser::BindingType::VALUE},
            },
            {},
            {},
            NilSpec,
            lyric_object::AccessType::Public);
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareMethodResult.getResult()));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<uint32_t>(StdCollectionsTrap::VECTOR_APPEND));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareMethodResult = VectorClass->declareMethod("replace",
            {
                {{}, "index", "", IntSpec, lyric_parser::BindingType::VALUE},
                {{}, "value", "", TSpec, lyric_parser::BindingType::VALUE},
            },
            {},
            {},
            TSpec,
            lyric_object::AccessType::Public);
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareMethodResult.getResult()));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<uint32_t>(StdCollectionsTrap::VECTOR_UPDATE));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareMethodResult = VectorClass->declareMethod("remove",
            {
                {{}, "index", "", IntSpec, lyric_parser::BindingType::VALUE},
            },
            {},
            {},
            TSpec,
            lyric_object::AccessType::Public);
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareMethodResult.getResult()));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<uint32_t>(StdCollectionsTrap::VECTOR_REMOVE));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareMethodResult = VectorClass->declareMethod("clear",
            {},
            {},
            {},
            NilSpec,
            lyric_object::AccessType::Public);
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareMethodResult.getResult()));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<uint32_t>(StdCollectionsTrap::VECTOR_CLEAR));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    lyric_common::TypeDef vectorIterableImplType;
    TU_ASSIGN_OR_RETURN (vectorIterableImplType, VectorClass->declareImpl(IterableTSpec));
    auto *VectorIterableImpl = VectorClass->getImpl(vectorIterableImplType);
    TU_ASSERT (VectorIterableImpl != nullptr);

    {
        auto declareExtensionResult = VectorIterableImpl->declareExtension("Iterate",
            {},
            {},
            {},
            IteratorTSpec);
        auto extension = declareExtensionResult.getResult();
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(extension.methodCall));
        auto *code = call->callProc()->procCode();
        code->loadClass(VectorIteratorClass->getAddress());
        code->trap(static_cast<uint32_t>(StdCollectionsTrap::VECTOR_ITERATE));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return lyric_assembler::AssemblerStatus::ok();
}
