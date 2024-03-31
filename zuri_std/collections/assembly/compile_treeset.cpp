/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_typing/callsite_reifier.h>
#include <zuri_std_collections/lib_types.h>

#include "compile_treeset.h"

tempo_utils::Result<lyric_assembler::ClassSymbol *>
declare_std_collections_TreeSet(
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
    TParam.variance = lyric_object::VarianceType::Invariant;

    auto declareTreeSetClassResult = block->declareClass("TreeSet",
        ObjectClass, lyric_object::AccessType::Public, {TParam});
    if (declareTreeSetClassResult.isStatus())
        return declareTreeSetClassResult.getStatus();
    auto *TreeSetClass = cast_symbol_to_class(symbolCache->getSymbol(declareTreeSetClassResult.getResult()));
    return TreeSetClass;
}

tempo_utils::Status
build_std_collections_TreeSet(
    lyric_assembler::ClassSymbol *TreeSetClass,
    lyric_assembler::ClassSymbol *TreeSetIteratorClass,
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *parentBlock,
    lyric_typing::TypeSystem *typeSystem)
{
    auto *fundamentalCache = state.fundamentalCache();
    auto *symbolCache = state.symbolCache();

    auto TSpec = lyric_parser::Assignable::forSingular({"T"});
    auto IntSpec = lyric_parser::Assignable::forSingular({"Int"});
    auto BoolSpec = lyric_parser::Assignable::forSingular({"Bool"});
    auto NilSpec = lyric_parser::Assignable::forSingular({"Nil"});
    auto IterableTSpec = lyric_parser::Assignable::forSingular({"Iterable"}, {TSpec});
    auto IteratorTSpec = lyric_parser::Assignable::forSingular({"Iterator"}, {TSpec});
    auto OrderedTSpec = lyric_parser::Assignable::forSingular({"Ordered"}, {
        lyric_parser::Assignable::forSingular({"T"})});
    auto orderedUrl = fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Ordered);

    lyric_assembler::CallAddress compareAddress;
    {
        auto declareFunctionResult = parentBlock->declareFunction("TreeSet.$compare",
            {
                {{}, "x1", "", TSpec, lyric_parser::BindingType::VALUE},
                {{}, "x2", "", TSpec, lyric_parser::BindingType::VALUE},
            },
            {},
            {
                {{}, "ord", "", OrderedTSpec, lyric_parser::BindingType::VALUE},
            },
            IntSpec,
            lyric_object::AccessType::Public,
            {
                {"T", 0, {}, lyric_object::VarianceType::Invariant, lyric_object::BoundType::None},
            });

        if (declareFunctionResult.isStatus())
            return declareFunctionResult.getStatus();
        auto functionUrl = declareFunctionResult.getResult();

        auto *call = cast_symbol_to_call(symbolCache->getSymbol(functionUrl));
        auto *block = call->callProc()->procBlock();
        tempo_utils::Status status;

        // push ord receiver onto the stack
        auto ordResult = block->resolveBinding("ord");
        if (ordResult.isStatus())
            return ordResult.getStatus();
        auto ord = ordResult.getResult();
        status = block->load(ord);
        if (!status.isOk())
            return status;

        // push Ordered concept descriptor onto the stack
        auto *sym = symbolCache->getSymbol(orderedUrl);
        if (sym == nullptr)
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "missing Ordered concept");
        if (sym->getSymbolType() != lyric_assembler::SymbolType::CONCEPT)
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid Ordered concept");
        auto *orderedSymbol = cast_symbol_to_concept(sym);
        auto resolveCompareResult = orderedSymbol->resolveAction("compare", ord.type);
        if (resolveCompareResult.isStatus())
            return resolveCompareResult.getStatus();
        auto compare = resolveCompareResult.getResult();

        lyric_typing::CallsiteReifier reifier(compare.getParameters(), compare.getRest(),
            compare.getTemplateUrl(), compare.getTemplateParameters(),
            compare.getTemplateArguments(), typeSystem);

        // push x1 and x2 onto the stack
        auto x1Result = block->resolveBinding("x1");
        if (x1Result.isStatus())
            return x1Result.getStatus();
        auto x1 = x1Result.getResult();
        TU_RETURN_IF_NOT_OK (block->load(x1));
        TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(x1.type));

        auto x2Result = block->resolveBinding("x2");
        if (x2Result.isStatus())
            return x2Result.getStatus();
        auto x2 = x2Result.getResult();
        TU_RETURN_IF_NOT_OK (block->load(x2));
        TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(x2.type));

        // return result of ord.compare()
        auto invokeCompareResult = compare.invoke(block, reifier);
        if (invokeCompareResult.isStatus())
            return invokeCompareResult.getStatus();

        compareAddress = call->getAddress();
    }
    {
        lyric_assembler::ParameterSpec restSpec;
        restSpec.type = TSpec;
        restSpec.binding = lyric_parser::BindingType::VALUE;
        restSpec.name = {};
        auto declareCtorResult = TreeSetClass->declareCtor(
            {},
            Option<lyric_assembler::ParameterSpec>(restSpec),
            {
                {{}, "ord", "", OrderedTSpec, lyric_parser::BindingType::VALUE}
            },
            lyric_object::AccessType::Public,
            static_cast<uint32_t>(StdCollectionsTrap::TREESET_ALLOC));
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareCtorResult.getResult()));
        auto *code = call->callProc()->procCode();

        // push $compare call descriptor onto the stack
        auto status = code->loadCall(compareAddress);
        if (!status.isOk())
            return status;

        code->trap(static_cast<uint32_t>(StdCollectionsTrap::TREESET_CTOR));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareMethodResult = TreeSetClass->declareMethod("size",
            {},
            {},
            {},
            IntSpec,
            lyric_object::AccessType::Public);
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareMethodResult.getResult()));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<uint32_t>(StdCollectionsTrap::TREESET_SIZE));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareMethodResult = TreeSetClass->declareMethod("contains",
            {
                {{}, "value", "", TSpec, lyric_parser::BindingType::VALUE}
            },
            {},
            {},
            BoolSpec,
            lyric_object::AccessType::Public);
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareMethodResult.getResult()));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<uint32_t>(StdCollectionsTrap::TREESET_CONTAINS));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareMethodResult = TreeSetClass->declareMethod("add",
            {
                {{}, "value", "", TSpec, lyric_parser::BindingType::VALUE},
            },
            {},
            {},
            BoolSpec,
            lyric_object::AccessType::Public);
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareMethodResult.getResult()));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<uint32_t>(StdCollectionsTrap::TREESET_ADD));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareMethodResult = TreeSetClass->declareMethod("remove",
            {
                {{}, "value", "", TSpec, lyric_parser::BindingType::VALUE}
            },
            {},
            {},
            BoolSpec,
            lyric_object::AccessType::Public);
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareMethodResult.getResult()));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<uint32_t>(StdCollectionsTrap::TREESET_REMOVE));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareMethodResult = TreeSetClass->declareMethod("clear",
            {},
            {},
            {},
            NilSpec,
            lyric_object::AccessType::Public);
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareMethodResult.getResult()));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<uint32_t>(StdCollectionsTrap::TREESET_CLEAR));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    lyric_common::TypeDef treesetIterableImplType;
    TU_ASSIGN_OR_RETURN (treesetIterableImplType, TreeSetClass->declareImpl(IterableTSpec));
    auto *TreesetIterableImpl = TreeSetClass->getImpl(treesetIterableImplType);
    TU_ASSERT (TreesetIterableImpl != nullptr);

    {
        auto declareExtensionResult = TreesetIterableImpl->declareExtension("Iterate",
            {},
            {},
            {},
            IteratorTSpec);
        auto extension = declareExtensionResult.getResult();
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(extension.methodCall));
        auto *code = call->callProc()->procCode();
        code->loadClass(TreeSetIteratorClass->getAddress());
        code->trap(static_cast<uint32_t>(StdCollectionsTrap::TREESET_ITERATE));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return lyric_assembler::AssemblerStatus::ok();
}
