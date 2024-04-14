/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
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

    auto resolveObjectResult = parentBlock->resolveClass(
        lyric_parser::Assignable::forSingular({"Object"}));
    if (resolveObjectResult.isStatus())
        return resolveObjectResult.getStatus();
    auto *ObjectClass = cast_symbol_to_class(symbolCache->getSymbol(resolveObjectResult.getResult()));

    auto KSpec = lyric_parser::Assignable::forSingular({"K"});
    auto VSpec = lyric_parser::Assignable::forSingular({"V"});
    auto IntSpec = lyric_parser::Assignable::forSingular({"Int"});
    auto BoolSpec = lyric_parser::Assignable::forSingular({"Bool"});
    auto NilSpec = lyric_parser::Assignable::forSingular({"Nil"});
    auto KVTuple2Spec = lyric_parser::Assignable::forSingular({"Tuple2"}, {KSpec, VSpec});
    auto EqualityKSpec = lyric_parser::Assignable::forSingular({"Equality"}, {
        lyric_parser::Assignable::forSingular({"K"}),
        lyric_parser::Assignable::forSingular({"K"})});
    auto equalityUrl = fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Equality);

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

    lyric_assembler::CallAddress equalsAddress;
    {
        auto declareFunctionResult = parentBlock->declareFunction("HashMap.$equals",
            {
                {{}, "x1", "", KSpec, lyric_parser::BindingType::VALUE},
                {{}, "x2", "", KSpec, lyric_parser::BindingType::VALUE},
            },
            {},
            {
                {{},  "eq", "", EqualityKSpec, lyric_parser::BindingType::VALUE},
            },
            IntSpec,
            lyric_object::AccessType::Public,
            {
                KParam,
            });

        if (declareFunctionResult.isStatus())
            return declareFunctionResult.getStatus();
        auto functionUrl = declareFunctionResult.getResult();

        auto *call = cast_symbol_to_call(symbolCache->getSymbol(functionUrl));
        auto *block = call->callProc()->procBlock();
        tempo_utils::Status status;

        // push ord receiver onto the stack
        auto eqResult = block->resolveReference("eq");
        if (eqResult.isStatus())
            return eqResult.getStatus();
        auto eq = eqResult.getResult();
        status = block->load(eq);
        if (!status.isOk())
            return status;

        // push Ordered concept descriptor onto the stack
        auto *sym = symbolCache->getSymbol(equalityUrl);
        if (sym == nullptr)
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "missing Equality concept");
        if (sym->getSymbolType() != lyric_assembler::SymbolType::CONCEPT)
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid Equality concept");
        auto *equalitySymbol = cast_symbol_to_concept(sym);
        auto resolveEqualsResult = equalitySymbol->resolveAction("equals", eq.typeDef);
        if (resolveEqualsResult.isStatus())
            return resolveEqualsResult.getStatus();
        auto equals = resolveEqualsResult.getResult();

        auto callsiteTypeArguments = eq.typeDef.getConcreteArguments();
        lyric_typing::CallsiteReifier reifier(equals.getParameters(), equals.getRest(),
            equals.getTemplateUrl(), equals.getTemplateParameters(),
            std::vector<lyric_common::TypeDef>(callsiteTypeArguments.begin(), callsiteTypeArguments.end()),
            typeSystem);
        TU_RETURN_IF_NOT_OK (reifier.initialize());

        // push x1 and x2 onto the stack
        auto x1Result = block->resolveReference("x1");
        if (x1Result.isStatus())
            return x1Result.getStatus();
        auto x1 = x1Result.getResult();
        TU_RETURN_IF_NOT_OK (block->load(x1));
        TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(x1.typeDef));

        auto x2Result = block->resolveReference("x2");
        if (x2Result.isStatus())
            return x2Result.getStatus();
        auto x2 = x2Result.getResult();
        TU_RETURN_IF_NOT_OK (block->load(x2));
        TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(x2.typeDef));

        // return result of eq.equals()
        auto invokeEqualsResult = equals.invoke(block, reifier);
        if (invokeEqualsResult.isStatus())
            return invokeEqualsResult.getStatus();

        equalsAddress = call->getAddress();
    }

    auto declareHashMapClassResult = parentBlock->declareClass(
        "HashMap", ObjectClass, lyric_object::AccessType::Public,
        {KParam, VParam});
    if (declareHashMapClassResult.isStatus())
        return declareHashMapClassResult.getStatus();
    auto *HashMapClass = cast_symbol_to_class(symbolCache->getSymbol(declareHashMapClassResult.getResult()));

    {
        auto declareMethodResult = HashMapClass->declareMethod("size",
            {},
            {},
            {},
            IntSpec,
            lyric_object::AccessType::Public);
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareMethodResult.getResult()));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<uint32_t>(StdCollectionsTrap::HASHMAP_SIZE));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareMethodResult = HashMapClass->declareMethod("contains",
            {
                {{},"key", "", KSpec, lyric_parser::BindingType::VALUE}
            },
            {},
            {},
            BoolSpec,
            lyric_object::AccessType::Public);
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareMethodResult.getResult()));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<uint32_t>(StdCollectionsTrap::HASHMAP_CONTAINS));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareMethodResult = HashMapClass->declareMethod("get",
            {
                {{},"key", "", KSpec, lyric_parser::BindingType::VALUE}
            },
            {},
            {},
            VSpec,
            lyric_object::AccessType::Public);
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareMethodResult.getResult()));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<uint32_t>(StdCollectionsTrap::HASHMAP_GET));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareMethodResult = HashMapClass->declareMethod("put",
            {
                {{},"key", "", KSpec, lyric_parser::BindingType::VALUE},
                {{},"value", "", VSpec, lyric_parser::BindingType::VALUE},
            },
            {},
            {},
            VSpec,
            lyric_object::AccessType::Public);
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareMethodResult.getResult()));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<uint32_t>(StdCollectionsTrap::HASHMAP_PUT));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareMethodResult = HashMapClass->declareMethod("remove",
            {
                {{},"key", "", KSpec, lyric_parser::BindingType::VALUE}
            },
            {},
            {},
            VSpec,
            lyric_object::AccessType::Public);
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareMethodResult.getResult()));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<uint32_t>(StdCollectionsTrap::HASHMAP_REMOVE));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareMethodResult = HashMapClass->declareMethod("clear",
            {},
            {},
            {},
            NilSpec,
            lyric_object::AccessType::Public);
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareMethodResult.getResult()));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<uint32_t>(StdCollectionsTrap::HASHMAP_CLEAR));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::ParameterSpec restSpec;
        restSpec.type = KVTuple2Spec;
        restSpec.binding = lyric_parser::BindingType::VALUE;
        restSpec.name = {};
        auto declareCtorResult = HashMapClass->declareCtor(
            {},
            Option<lyric_assembler::ParameterSpec>(restSpec),
            {
                {{}, "eq", "", EqualityKSpec, lyric_parser::BindingType::VALUE}
            },
            lyric_object::AccessType::Public,
            static_cast<uint32_t>(StdCollectionsTrap::HASHMAP_ALLOC));
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareCtorResult.getResult()));
        auto *proc = call->callProc();
        auto *code = proc->procCode();

        // push $compare call descriptor onto the stack
        auto status = code->loadCall(equalsAddress);
        if (!status.isOk())
            return status;

        code->trap(static_cast<uint32_t>(StdCollectionsTrap::HASHMAP_CTOR));

        auto *procBlock = proc->procBlock();
        auto IntType = procBlock->resolveAssignable(IntSpec).orElseThrow();
        auto *Tuple2sym = symbolCache->getSymbol(fundamentalCache->getTupleUrl(2));
        TU_ASSERT (Tuple2sym != nullptr && Tuple2sym->getSymbolType() == lyric_assembler::SymbolType::CLASS);
        auto *Tuple2class = cast_symbol_to_class(Tuple2sym);
        auto t0 = Tuple2class->getMember("t0").getValue();
        auto t1 = Tuple2class->getMember("t1").getValue();
        auto putMethod = HashMapClass->getMethod("put").getValue();
        auto *putSym = symbolCache->getSymbol(putMethod.methodCall);
        TU_ASSERT (putSym != nullptr && putSym->getSymbolType() == lyric_assembler::SymbolType::CALL);
        auto *putCall = cast_symbol_to_call(putSym);
        auto putAddress = putCall->getAddress();

        // initialize counter to 0
        auto counter = procBlock->declareTemporary(IntType, lyric_parser::BindingType::VARIABLE).orElseThrow();
        code->loadInt(0);
        procBlock->store(counter);
        // top of the loop
        auto topLabel = code->makeLabel().orElseThrow();
        // check if counter is less than va size
        procBlock->load(counter);
        code->writeOpcode(lyric_object::Opcode::OP_VA_SIZE);
        code->writeOpcode(lyric_object::Opcode::OP_I64_CMP);
        // if counter equals va size then return
        auto patchIfRemaining = code->jumpIfLessThan().orElseThrow();
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
        // otherwise process next va item
        auto continueLabel = code->makeLabel().orElseThrow();
        code->patch(patchIfRemaining, continueLabel).orThrow();
        // load item and push item key onto stack
        procBlock->load(counter);
        code->writeOpcode(lyric_object::Opcode::OP_VA_LOAD);
        procBlock->load(t0);
        code->rdropValue(1);
        // load item and push item value onto stack
        procBlock->load(counter);
        code->writeOpcode(lyric_object::Opcode::OP_VA_LOAD);
        procBlock->load(t1);
        code->rdropValue(1);
        // call put
        code->loadSynthetic(lyric_assembler::SyntheticType::THIS);
        code->callVirtual(putAddress, 2, lyric_object::CALL_RECEIVER_FOLLOWS);
        code->popValue();
        // increment counter
        procBlock->load(counter);
        code->loadInt(1);
        code->writeOpcode(lyric_object::Opcode::OP_I64_ADD);
        procBlock->store(counter);
        // jump unconditionally to the top of the loop
        code->jump(topLabel).orThrow();
    }

    return lyric_assembler::AssemblerStatus::ok();
}
