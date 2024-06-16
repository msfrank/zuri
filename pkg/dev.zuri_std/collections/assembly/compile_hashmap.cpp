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

#include "compile_hashmap.h"

tempo_utils::Status
build_std_collections_HashMap(
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *parentBlock,
    lyric_typing::TypeSystem *typeSystem)
{
    auto *fundamentalCache = state.fundamentalCache();
    auto *symbolCache = state.symbolCache();
    auto *typeCache = state.typeCache();

    auto resolveObjectResult = parentBlock->resolveClass(
        lyric_parser::Assignable::forSingular({"Object"}));
    if (resolveObjectResult.isStatus())
        return resolveObjectResult.getStatus();
    auto *ObjectClass = cast_symbol_to_class(
        symbolCache->getOrImportSymbol(resolveObjectResult.getResult()).orElseThrow());

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

    auto equalityUrl = fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Equality);

    lyric_assembler::CallAddress equalsAddress;
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, parentBlock->declareFunction(
            "HashMap.$equals", lyric_object::AccessType::Public, {KParam}));
        auto *templateHandle = callSymbol->callTemplate();
        auto KType = templateHandle->getPlaceholder("K");
        lyric_assembler::TypeHandle *equalityKHandle;
        TU_ASSIGN_OR_RETURN (equalityKHandle, typeCache->declareParameterizedType(equalityUrl, {KType, KType}));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("x1", "", KType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("x2", "", KType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendCtxParameter("eq", "", equalityKHandle->getTypeDef()));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, IntType));
        auto *block = procHandle->procBlock();

        // push eq receiver onto the stack
        lyric_assembler::DataReference eq;
        TU_ASSIGN_OR_RETURN (eq, block->resolveReference("eq"));
        TU_RETURN_IF_NOT_OK (block->load(eq));

        // push Equality concept descriptor onto the stack
        lyric_assembler::AbstractSymbol *sym;
        TU_ASSIGN_OR_RETURN (sym, symbolCache->getOrImportSymbol(equalityUrl));
        auto *equalitySymbol = cast_symbol_to_concept(sym);
        lyric_assembler::CallableInvoker invoker;
        TU_RETURN_IF_NOT_OK (equalitySymbol->prepareAction("equals", eq.typeDef, invoker));

        auto eqTypeArguments = eq.typeDef.getConcreteArguments();
        std::vector<lyric_common::TypeDef> callsiteTypeArguments(eqTypeArguments.begin(), eqTypeArguments.end());
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

        // return result of eq.equals()
        TU_RETURN_IF_STATUS (invoker.invoke(block, reifier));

        equalsAddress = callSymbol->getAddress();
    }

    auto declareHashMapClassResult = parentBlock->declareClass(
        "HashMap", ObjectClass, lyric_object::AccessType::Public,
        {KParam, VParam});
    if (declareHashMapClassResult.isStatus())
        return declareHashMapClassResult.getStatus();
    auto *HashMapClass = cast_symbol_to_class(
        symbolCache->getOrImportSymbol(declareHashMapClassResult.getResult()).orElseThrow());

    auto *templateHandle = HashMapClass->classTemplate();
    auto KType = templateHandle->getPlaceholder("K");
    auto VType = templateHandle->getPlaceholder("V");

    lyric_assembler::TypeHandle *tuple2KVHandle;
    TU_ASSIGN_OR_RETURN (tuple2KVHandle, typeCache->declareParameterizedType(
        fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Tuple2), {KType, VType}));
    auto Tuple2KVType = tuple2KVHandle->getTypeDef();

    lyric_assembler::TypeHandle *equalityKHandle;
    TU_ASSIGN_OR_RETURN (equalityKHandle, typeCache->declareParameterizedType(equalityUrl, {KType, KType}));
    auto EqualityKType = equalityKHandle->getTypeDef();

    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, HashMapClass->declareMethod(
            "Size", lyric_object::AccessType::Public));
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall({}, IntType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdCollectionsTrap::HASHMAP_SIZE));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, HashMapClass->declareMethod(
            "Contains", lyric_object::AccessType::Public));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("key", "", KType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, BoolType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdCollectionsTrap::HASHMAP_CONTAINS));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, HashMapClass->declareMethod(
            "Get", lyric_object::AccessType::Public));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("key", "", KType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, VType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdCollectionsTrap::HASHMAP_GET));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, HashMapClass->declareMethod(
            "Put", lyric_object::AccessType::Public));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("key", "", KType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("value", "", VType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, VType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdCollectionsTrap::HASHMAP_PUT));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, HashMapClass->declareMethod(
            "Remove", lyric_object::AccessType::Public));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("key", "", KType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, VType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdCollectionsTrap::HASHMAP_REMOVE));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, HashMapClass->declareMethod(
            "Clear", lyric_object::AccessType::Public));
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall({}, NilType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdCollectionsTrap::HASHMAP_CLEAR));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, HashMapClass->declareCtor(
            lyric_object::AccessType::Public, static_cast<tu_uint32>(StdCollectionsTrap::HASHMAP_ALLOC)));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendRestParameter("", Tuple2KVType));
        TU_RETURN_IF_NOT_OK (packBuilder.appendCtxParameter("eq", "", EqualityKType));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, lyric_common::TypeDef::noReturn()));
        auto *codeBuilder = procHandle->procCode();

        // push HashMap.$equals call descriptor onto the stack
        auto status = codeBuilder->loadCall(equalsAddress);
        if (!status.isOk())
            return status;

        codeBuilder->trap(static_cast<tu_uint32>(StdCollectionsTrap::HASHMAP_CTOR));

        auto *procBlock = procHandle->procBlock();
        auto *Tuple2sym = symbolCache->getOrImportSymbol(fundamentalCache->getTupleUrl(2)).orElseThrow();
        TU_ASSERT (Tuple2sym != nullptr && Tuple2sym->getSymbolType() == lyric_assembler::SymbolType::CLASS);
        auto *Tuple2class = cast_symbol_to_class(Tuple2sym);
        auto t0 = Tuple2class->getMember("t0").getValue();
        auto t1 = Tuple2class->getMember("t1").getValue();
        auto putMethod = HashMapClass->getMethod("Put").getValue();
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
