/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <zuri_std_collections/lib_types.h>

#include "compile_vector.h"

tempo_utils::Status
build_std_collections_Vector(
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *parentBlock,
    lyric_typing::TypeSystem *typeSystem)
{
    auto *symbolCache = state.symbolCache();

    auto resolveObjectResult = parentBlock->resolveClass(
        lyric_parser::Assignable::forSingular({"Object"}));
    if (resolveObjectResult.isStatus())
        return resolveObjectResult.getStatus();
    auto *ObjectClass = cast_symbol_to_class(symbolCache->getSymbol(resolveObjectResult.getResult()));

    lyric_object::TemplateParameter TParam;
    TParam.name = "T";
    TParam.index = 0;
    TParam.typeDef = {};
    TParam.bound = lyric_object::BoundType::None;
    TParam.variance = lyric_object::VarianceType::Covariant;

    auto declareVectorClassResult = parentBlock->declareClass("Vector",
        ObjectClass, lyric_object::AccessType::Public, {TParam});
    if (declareVectorClassResult.isStatus())
        return declareVectorClassResult.getStatus();
    auto *VectorClass = cast_symbol_to_class(symbolCache->getSymbol(declareVectorClassResult.getResult()));

    auto TSpec = lyric_parser::Assignable::forSingular({"T"});
    auto IntSpec = lyric_parser::Assignable::forSingular({"Int"});
    auto NilSpec = lyric_parser::Assignable::forSingular({"Nil"});
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
    {
        auto declareMethodResult = VectorClass->declareMethod("iter",
            {},
            {},
            {},
            IteratorTSpec,
            lyric_object::AccessType::Public);
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareMethodResult.getResult()));
        auto *code = call->callProc()->procCode();
        auto resolveIteratorClassResult = parentBlock->resolveClass(IteratorTSpec);
        if (resolveIteratorClassResult.isStatus())
            return resolveIteratorClassResult.getStatus();
        auto *iteratorClass = cast_symbol_to_class(symbolCache->getSymbol(resolveIteratorClassResult.getResult()));
        code->loadClass(iteratorClass->getAddress());
        code->trap(static_cast<uint32_t>(StdCollectionsTrap::VECTOR_ITER));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return lyric_assembler::AssemblerStatus::ok();
}
