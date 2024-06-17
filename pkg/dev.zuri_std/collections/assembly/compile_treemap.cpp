/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/pack_builder.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/template_handle.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_typing/callsite_reifier.h>
#include <zuri_std_collections/lib_types.h>

#include "compile_treemap.h"

tempo_utils::Status
build_std_collections_TreeMap(
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *parentBlock,
    lyric_typing::TypeSystem *typeSystem)
{
    auto *fundamentalCache = state.fundamentalCache();
    auto *symbolCache = state.symbolCache();
    auto *typeCache = state.typeCache();

    lyric_assembler::ClassSymbol *ObjectClass;
    TU_ASSIGN_OR_RETURN (ObjectClass, parentBlock->resolveClass(
        fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object)));

    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    auto BoolType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool);
    auto NilType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Nil);

    lyric_object::TemplateParameter KParam;
    KParam.name = "K";
    KParam.index = 0;
    KParam.typeDef = state.fundamentalCache()->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
    KParam.bound = lyric_object::BoundType::None;
    KParam.variance = lyric_object::VarianceType::Invariant;

    lyric_object::TemplateParameter VParam;
    VParam.name = "V";
    VParam.index = 1;
    VParam.typeDef = state.fundamentalCache()->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
    VParam.bound = lyric_object::BoundType::None;
    VParam.variance = lyric_object::VarianceType::Covariant;

    auto orderedUrl = fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Ordered);

    lyric_assembler::CallAddress compareAddress;
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, parentBlock->declareFunction(
            "TreeMap.$compare", lyric_object::AccessType::Public, {KParam}));
        auto *templateHandle = callSymbol->callTemplate();
        auto KType = templateHandle->getPlaceholder("K");
        lyric_assembler::TypeHandle *orderedKHandle;
        TU_ASSIGN_OR_RETURN (orderedKHandle, typeCache->declareParameterizedType(orderedUrl, {KType}));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("x1", "", KType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("x2", "", KType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendCtxParameter("ord", "", orderedKHandle->getTypeDef()));
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

    auto declareTreeMapClassResult = parentBlock->declareClass("TreeMap", ObjectClass,
        lyric_object::AccessType::Public, {KParam, VParam});
    if (declareTreeMapClassResult.isStatus())
        return declareTreeMapClassResult.getStatus();
    auto *TreeMapClass = cast_symbol_to_class(
        symbolCache->getOrImportSymbol(declareTreeMapClassResult.getResult()).orElseThrow());

    auto *templateHandle = TreeMapClass->classTemplate();
    auto KType = templateHandle->getPlaceholder("K");
    auto VType = templateHandle->getPlaceholder("V");

    lyric_assembler::TypeHandle *tuple2KVHandle;
    TU_ASSIGN_OR_RETURN (tuple2KVHandle, typeCache->declareParameterizedType(
        fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Tuple2), {KType, VType}));
    auto Tuple2KVType = tuple2KVHandle->getTypeDef();

    lyric_assembler::TypeHandle *orderedKHandle;
    TU_ASSIGN_OR_RETURN (orderedKHandle, typeCache->declareParameterizedType(orderedUrl, {KType}));
    auto OrderedKType = orderedKHandle->getTypeDef();

    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, TreeMapClass->declareMethod(
            "Size", lyric_object::AccessType::Public));
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall({}, IntType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdCollectionsTrap::TREEMAP_SIZE));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, TreeMapClass->declareMethod(
            "Contains", lyric_object::AccessType::Public));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("key", "", KType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, BoolType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdCollectionsTrap::TREEMAP_CONTAINS));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, TreeMapClass->declareMethod(
            "Get", lyric_object::AccessType::Public));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("key", "", KType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, VType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdCollectionsTrap::TREEMAP_GET));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, TreeMapClass->declareMethod(
            "Put", lyric_object::AccessType::Public));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("key", "", KType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("value", "", VType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, VType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdCollectionsTrap::TREEMAP_PUT));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, TreeMapClass->declareMethod(
            "Remove", lyric_object::AccessType::Public));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("key", "", KType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, VType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdCollectionsTrap::TREEMAP_REMOVE));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, TreeMapClass->declareMethod(
            "Clear", lyric_object::AccessType::Public));
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall({}, NilType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdCollectionsTrap::TREEMAP_CLEAR));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, TreeMapClass->declareCtor(
            lyric_object::AccessType::Public, static_cast<tu_uint32>(StdCollectionsTrap::TREEMAP_ALLOC)));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendRestParameter("", Tuple2KVType));
        TU_RETURN_IF_NOT_OK (packBuilder.appendCtxParameter("ord", "", OrderedKType));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, lyric_common::TypeDef::noReturn()));
        auto *codeBuilder = procHandle->procCode();

        // push TreeMap.$compare call descriptor onto the stack
        auto status = codeBuilder->loadCall(compareAddress);
        if (!status.isOk())
            return status;

        codeBuilder->trap(static_cast<tu_uint32>(StdCollectionsTrap::TREEMAP_CTOR));

        auto *procBlock = procHandle->procBlock();
        auto *Tuple2sym = symbolCache->getOrImportSymbol(fundamentalCache->getTupleUrl(2)).orElseThrow();
        TU_ASSERT (Tuple2sym != nullptr && Tuple2sym->getSymbolType() == lyric_assembler::SymbolType::CLASS);
        auto *Tuple2class = cast_symbol_to_class(Tuple2sym);
        auto t0 = Tuple2class->getMember("t0").getValue();
        auto t1 = Tuple2class->getMember("t1").getValue();
        auto putMethod = TreeMapClass->getMethod("Put").getValue();
        auto *putSym = symbolCache->getOrImportSymbol(putMethod.methodCall).orElseThrow();
        TU_ASSERT (putSym != nullptr && putSym->getSymbolType() == lyric_assembler::SymbolType::CALL);
        auto *putCall = cast_symbol_to_call(putSym);
        auto putAddress = putCall->getAddress();

        // initialize counter to 0
        auto counter = procBlock->declareTemporary(IntType, lyric_parser::BindingType::VARIABLE).orElseThrow();
        codeBuilder->loadInt(0);
        procBlock->store(counter);
        // top of the loop
        auto topLabel = codeBuilder->makeLabel().orElseThrow();
        // check if counter is less than va size
        procBlock->load(counter);
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_VA_SIZE);
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_I64_CMP);
        // if counter equals va size then return
        auto patchIfRemaining = codeBuilder->jumpIfLessThan().orElseThrow();
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
        // otherwise process next va item
        auto continueLabel = codeBuilder->makeLabel().orElseThrow();
        codeBuilder->patch(patchIfRemaining, continueLabel).orThrow();
        // load item and push item key onto stack
        procBlock->load(counter);
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_VA_LOAD);
        procBlock->load(t0);
        codeBuilder->rdropValue(1);
        // load item and push item value onto stack
        procBlock->load(counter);
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_VA_LOAD);
        procBlock->load(t1);
        codeBuilder->rdropValue(1);
        // call put
        codeBuilder->loadSynthetic(lyric_assembler::SyntheticType::THIS);
        codeBuilder->callVirtual(putAddress, 2, lyric_object::CALL_RECEIVER_FOLLOWS);
        codeBuilder->popValue();
        // increment counter
        procBlock->load(counter);
        codeBuilder->loadInt(1);
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_I64_ADD);
        procBlock->store(counter);
        // jump unconditionally to the top of the loop
        codeBuilder->jump(topLabel).orThrow();
    }

    return {};
}
