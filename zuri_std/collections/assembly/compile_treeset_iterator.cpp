/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <zuri_std_collections/lib_types.h>

#include "compile_treeset_iterator.h"

tempo_utils::Result<lyric_assembler::ClassSymbol *>
declare_std_collections_TreeSetIterator(
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
    TParam.typeDef = {};
    TParam.bound = lyric_object::BoundType::None;
    TParam.variance = lyric_object::VarianceType::Contravariant;

    auto declareTreeSetIteratorClassResult = block->declareClass("TreeSetIterator",
        ObjectClass, lyric_object::AccessType::Public, {TParam});
    if (declareTreeSetIteratorClassResult.isStatus())
        return declareTreeSetIteratorClassResult.getStatus();
    auto *TreeSetIteratorClass = cast_symbol_to_class(symbolCache->getSymbol(declareTreeSetIteratorClassResult.getResult()));
    return TreeSetIteratorClass;
}

tempo_utils::Status
build_std_collections_TreeSetIterator(
    lyric_assembler::ClassSymbol *TreeSetIteratorClass,
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem)
{
    auto *symbolCache = state.symbolCache();

    auto TSpec = lyric_parser::Assignable::forSingular({"T"});
    auto BoolSpec = lyric_parser::Assignable::forSingular({"Bool"});
    auto IteratorTSpec = lyric_parser::Assignable::forSingular({"Iterator"}, {TSpec});

    {
        lyric_assembler::ParameterSpec restSpec;
        restSpec.type = TSpec;
        restSpec.binding = lyric_parser::BindingType::VALUE;
        restSpec.name = {};
        auto declareCtorResult = TreeSetIteratorClass->declareCtor(
            {},
            Option<lyric_assembler::ParameterSpec>(restSpec),
            {},
            lyric_object::AccessType::Public,
            static_cast<uint32_t>(StdCollectionsTrap::TREESET_ITERATOR_ALLOC));
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareCtorResult.getResult()));
        auto *code = call->callProc()->procCode();
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    lyric_common::TypeDef iteratorImplType;
    TU_ASSIGN_OR_RETURN (iteratorImplType, TreeSetIteratorClass->declareImpl(IteratorTSpec));
    auto *IteratorImpl = TreeSetIteratorClass->getImpl(iteratorImplType);
    TU_ASSERT (IteratorImpl != nullptr);

    {
        auto declareExtensionResult = IteratorImpl->declareExtension("Valid",
            {},
            {},
            {},
            BoolSpec);
        auto extension = declareExtensionResult.getResult();
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(extension.methodCall));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<uint32_t>(StdCollectionsTrap::TREESET_ITERATOR_VALID));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareExtensionResult = IteratorImpl->declareExtension("Next",
            {},
            {},
            {},
            TSpec);
        auto extension = declareExtensionResult.getResult();
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(extension.methodCall));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<uint32_t>(StdCollectionsTrap::TREESET_ITERATOR_NEXT));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return lyric_assembler::AssemblerStatus::ok();
}
