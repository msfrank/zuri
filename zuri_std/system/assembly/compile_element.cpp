/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_typing/callsite_reifier.h>
#include <zuri_std_system/lib_types.h>

#include "compile_element.h"

tempo_utils::Result<lyric_assembler::StructSymbol *>
declare_std_system_Element(
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block)
{
    auto *symbolCache = state.symbolCache();

    auto resolveRecordResult = block->resolveStruct(
        lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"Record"})));
    if (resolveRecordResult.isStatus())
        return resolveRecordResult.getStatus();
    auto *RecordStruct = cast_symbol_to_struct(symbolCache->getSymbol(resolveRecordResult.getResult()));

    auto declareElementStructResult = block->declareStruct("Element",
        RecordStruct, lyric_object::AccessType::Public, lyric_object::DeriveType::Any);
    if (declareElementStructResult.isStatus())
        return declareElementStructResult.getStatus();
    auto *ElementStruct = cast_symbol_to_struct(symbolCache->getSymbol(declareElementStructResult.getResult()));
    return ElementStruct;
}

tempo_utils::Status
build_std_system_Element(
    lyric_assembler::StructSymbol *ElementStruct,
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem)
{
    auto *symbolCache = state.symbolCache();

    auto UrlSpec = lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"Url"}));
    auto IntSpec = lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"Int"}));
    auto ValueSpec = lyric_parser::Assignable::forUnion({
        lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"Intrinsic"})),
        lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"String"})),
        lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"Url"})),
        lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"Attr"})),
        lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"Element"})),
    });
    auto SeqSpec = lyric_parser::Assignable::forSingular(lyric_common::SymbolPath({"Seq"}));
    auto IteratorTSpec = lyric_parser::Assignable::forSingular(
        lyric_common::SymbolPath({"Iterator"}), {ValueSpec});

    auto declareNsResult = ElementStruct->declareMember("ns", UrlSpec);
    if (declareNsResult.isStatus())
        return declareNsResult.getStatus();
    auto *NsField = static_cast<lyric_assembler::FieldSymbol *>(
        symbolCache->getSymbol(declareNsResult.getResult().symbol));

    auto declareIdResult = ElementStruct->declareMember("id", IntSpec);
    if (declareIdResult.isStatus())
        return declareIdResult.getStatus();
    auto *IdField = static_cast<lyric_assembler::FieldSymbol *>(
        symbolCache->getSymbol(declareIdResult.getResult().symbol));

    auto declareChildrenResult = ElementStruct->declareMember("children", SeqSpec);
    if (declareChildrenResult.isStatus())
        return declareChildrenResult.getStatus();
    auto *ChildrenField = static_cast<lyric_assembler::FieldSymbol *>(
        symbolCache->getSymbol(declareChildrenResult.getResult().symbol));

    {
        auto *SuperStruct = ElementStruct->superStruct();
        auto superCtorUrl = SuperStruct->getCtor();
        if (!symbolCache->hasSymbol(superCtorUrl))
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "missing ctor for Element");
        auto *superCtorSym = symbolCache->getSymbol(superCtorUrl);
        if (superCtorSym->getSymbolType() != lyric_assembler::SymbolType::CALL)
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid ctor for Element");
        auto *superCtorCall = cast_symbol_to_call(superCtorSym);
        superCtorCall->touch();

        auto declareCtorResult = ElementStruct->declareCtor(
            {
                {{}, "ns", "ns", UrlSpec, lyric_parser::BindingType::VALUE},
                {{}, "id", "id", IntSpec, lyric_parser::BindingType::VALUE},
            },
            Option<lyric_assembler::ParameterSpec>({{}, "children", "", ValueSpec, lyric_parser::BindingType::VALUE}),
            lyric_object::AccessType::Public,
            static_cast<uint32_t>(StdSystemTrap::ELEMENT_ALLOC));
        auto *call = static_cast<lyric_assembler::CallSymbol *>(symbolCache->getSymbol(declareCtorResult.getResult()));
        auto *code = call->callProc()->procCode();
        auto *ctorBlock = call->callProc()->procBlock();

        // call the super constructor
        code->loadSynthetic(lyric_assembler::SyntheticType::THIS);
        code->callVirtual(superCtorCall->getAddress(), 0);

        // load the ns argument and store it in member
        code->loadArgument(lyric_assembler::ArgumentOffset(0));
        code->storeField(NsField->getAddress());

        // load the id argument and store it in member
        code->loadArgument(lyric_assembler::ArgumentOffset(1));
        code->storeField(IdField->getAddress());

        auto resolveSeqResult = block->resolveStruct(SeqSpec);
        if (resolveSeqResult.isStatus())
            return resolveSeqResult.getStatus();
        auto *SeqStruct = static_cast<lyric_assembler::StructSymbol *>(
            symbolCache->getSymbol(resolveSeqResult.getResult()));
        auto resolveCtorResult = SeqStruct->resolveCtor();
        if (resolveCtorResult.isStatus())
            return resolveCtorResult.getStatus();
        auto ctor = resolveCtorResult.getResult();
        lyric_typing::CallsiteReifier ctorReifier(ctor.getParameters(), ctor.getRest(),
            ctor.getTemplateUrl(), ctor.getTemplateParameters(), {}, typeSystem);

        // FIXME: how do we reify the forwarded rest arg?
        //auto status = ctorReifier.reifyNextArgument(call->getRest().getValue().type);
        //if (status.notOk())
        //    return status;
        auto invokeNewResult = ctor.invokeNew(ctorBlock, ctorReifier, lyric_object::CALL_FORWARD_REST);
        if (invokeNewResult.isStatus())
            return invokeNewResult.getStatus();

        // seq is on the top of the stack, store it as the children member
        code->storeField(ChildrenField->getAddress());

        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareMethodResult = ElementStruct->declareMethod("$getChildrenIterator",
            {},
            {},
            {},
            IteratorTSpec);
        if (declareMethodResult.isStatus())
            return declareMethodResult.getStatus();
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(declareMethodResult.getResult()));
        auto *proc = call->callProc();
        auto *code = proc->procCode();
        auto *methodBlock = proc->procBlock();

        // load children member onto top of the stack
        code->loadSynthetic(lyric_assembler::SyntheticType::THIS);
        code->loadField(ChildrenField->getAddress());

        // call 'Iter' method on children
        auto resolveStructResult = methodBlock->resolveStruct(SeqSpec);
        if (resolveStructResult.isStatus())
            return resolveStructResult.getStatus();
        auto *SeqStruct = cast_symbol_to_struct(symbolCache->getSymbol(resolveStructResult.getResult()));
        auto resolveIterResult = SeqStruct->resolveMethod("Iter", SeqStruct->getAssignableType());
        if (resolveIterResult.isStatus())
            return resolveIterResult.getStatus();
        auto iter = resolveIterResult.getResult();

        lyric_typing::CallsiteReifier iterReifier(iter.getParameters(), iter.getRest(),
            iter.getTemplateUrl(), iter.getTemplateParameters(), {}, typeSystem);

        auto invokeNewResult = iter.invoke(methodBlock, iterReifier);
        if (invokeNewResult.isStatus())
            return invokeNewResult.getStatus();
    }

    return tempo_utils::Status();
}