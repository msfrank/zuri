/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_typing/callsite_reifier.h>
#include <zuri_std_system/lib_types.h>

#include "compile_operation.h"

tempo_utils::Result<lyric_assembler::StructSymbol *>
build_std_system_Operation(
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem)
{
    auto *symbolCache = state.symbolCache();

    auto resolveRecordResult = block->resolveStruct(
        lyric_parser::Assignable::forSingular({"Record"}));
    if (resolveRecordResult.isStatus())
        return resolveRecordResult.getStatus();
    auto *RecordStruct = cast_symbol_to_struct(
        symbolCache->getOrImportSymbol(resolveRecordResult.getResult()).orElseThrow());

    auto declareOperationStructResult = block->declareStruct("Operation",
        RecordStruct, lyric_object::AccessType::Public, lyric_object::DeriveType::Sealed, true);
    if (declareOperationStructResult.isStatus())
        return declareOperationStructResult.getStatus();
    auto *OperationStruct = cast_symbol_to_struct(
        symbolCache->getOrImportSymbol(declareOperationStructResult.getResult()).orElseThrow());

    auto IntSpec = lyric_parser::Assignable::forSingular({"Int"});
    auto StringSpec = lyric_parser::Assignable::forSingular({"String"});
    auto UrlSpec = lyric_parser::Assignable::forSingular({"Url"});
    auto AttrSpec = lyric_parser::Assignable::forSingular({"Attr"});
    auto ElementSpec = lyric_parser::Assignable::forSingular({"Element"});

    auto ValueSpec = lyric_parser::Assignable::forUnion({
        lyric_parser::Assignable::forSingular({"Intrinsic"}),
        lyric_parser::Assignable::forSingular({"Element"}),
    });

    {
        auto declareCtorResult = OperationStruct->declareCtor(
            {},
            {},
            lyric_object::AccessType::Public);
        auto *call = cast_symbol_to_call(symbolCache->getOrImportSymbol(declareCtorResult.getResult()).orElseThrow());
        auto *code = call->callProc()->procCode();
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareFunctionResult = block->declareFunction("Operation.$createAttr",
            {
            {{}, "ns", "", UrlSpec, lyric_parser::BindingType::VALUE},
            {{}, "id", "", IntSpec, lyric_parser::BindingType::VALUE},
            {{}, "value", "", ValueSpec, lyric_parser::BindingType::VALUE},
            },
            {},
            {},
            AttrSpec,
            lyric_object::AccessType::Public,
            {});
        if (declareFunctionResult.isStatus())
            return declareFunctionResult.getStatus();
        auto functionUrl = declareFunctionResult.getResult();
        auto *call = cast_symbol_to_call(symbolCache->getOrImportSymbol(functionUrl).orElseThrow());
        auto *proc = call->callProc();
        auto *code = proc->procCode();
        auto *createBlock = proc->procBlock();

        auto resolveStructResult = createBlock->resolveStruct(AttrSpec);
        if (resolveStructResult.isStatus())
            return resolveStructResult.getStatus();
        auto *AttrStruct = cast_symbol_to_struct(symbolCache->getOrImportSymbol(resolveStructResult.getResult()).orElseThrow());
        auto resolveCtorResult = AttrStruct->resolveCtor();
        if (resolveCtorResult.isStatus())
            return resolveCtorResult.getStatus();
        auto ctor = resolveCtorResult.getResult();
        lyric_typing::CallsiteReifier ctorReifier(ctor.getParameters(), ctor.getRest(),
            ctor.getTemplateUrl(), ctor.getTemplateParameters(), {}, typeSystem);
        TU_RETURN_IF_NOT_OK (ctorReifier.initialize());

        TU_RETURN_IF_NOT_OK (code->loadArgument(lyric_assembler::ArgumentOffset(0)));
        TU_RETURN_IF_NOT_OK (ctorReifier.reifyNextArgument(call->getParameters().at(0).typeDef));
        TU_RETURN_IF_NOT_OK (code->loadArgument(lyric_assembler::ArgumentOffset(1)));
        TU_RETURN_IF_NOT_OK (ctorReifier.reifyNextArgument(call->getParameters().at(1).typeDef));
        TU_RETURN_IF_NOT_OK (code->loadArgument(lyric_assembler::ArgumentOffset(2)));
        TU_RETURN_IF_NOT_OK (ctorReifier.reifyNextArgument(call->getParameters().at(2).typeDef));
        auto invokeNewResult = ctor.invokeNew(createBlock, ctorReifier);
        if (invokeNewResult.isStatus())
            return invokeNewResult.getStatus();
    }

    return OperationStruct;
}

tempo_utils::Status
build_std_system_AppendOperation(
    lyric_assembler::StructSymbol *OperationStruct,
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem)
{
    auto *symbolCache = state.symbolCache();

    auto declareAppendOperationStructResult = block->declareStruct("AppendOperation",
        OperationStruct, lyric_object::AccessType::Public, lyric_object::DeriveType::Final);
    if (declareAppendOperationStructResult.isStatus())
        return declareAppendOperationStructResult.getStatus();
    auto *AppendOperationStruct = cast_symbol_to_struct(
        symbolCache->getOrImportSymbol(declareAppendOperationStructResult.getResult()).orElseThrow());

    //
    OperationStruct->putSealedType(AppendOperationStruct->getAssignableType());

    auto StringSpec = lyric_parser::Assignable::forSingular({"String"});
    auto valueUnionSpec = lyric_parser::Assignable::forUnion({
            lyric_parser::Assignable::forSingular({"Intrinsic"}),
            lyric_parser::Assignable::forSingular({"Attr"}),
            lyric_parser::Assignable::forSingular({"Element"}),
        });

    // declare the "path" member
    auto declarePathMemberResult = AppendOperationStruct->declareMember("path", StringSpec);
    if (declarePathMemberResult.isStatus())
        return declarePathMemberResult.getStatus();
    auto *PathField = cast_symbol_to_field(
        symbolCache->getOrImportSymbol(declarePathMemberResult.getResult().symbolUrl).orElseThrow());

    // declare the "value" member
    auto declareValueMemberResult = AppendOperationStruct->declareMember("value", valueUnionSpec);
    if (declareValueMemberResult.isStatus())
        return declareValueMemberResult.getStatus();
    auto *ValueField = cast_symbol_to_field(
        symbolCache->getOrImportSymbol(declareValueMemberResult.getResult().symbolUrl).orElseThrow());

    {
        auto superCtorUrl = OperationStruct->getCtor();
        if (!symbolCache->hasSymbol(superCtorUrl))
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "missing ctor for AppendOperation");
        lyric_assembler::AbstractSymbol *superCtorSym;
        TU_ASSIGN_OR_RETURN (superCtorSym, symbolCache->getOrImportSymbol(superCtorUrl));
        if (superCtorSym->getSymbolType() != lyric_assembler::SymbolType::CALL)
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid ctor for AppendOperation");
        auto *superCtorCall = cast_symbol_to_call(superCtorSym);

        auto declareCtorResult = AppendOperationStruct->declareCtor(
            {
                {{}, "path", "path", StringSpec, lyric_parser::BindingType::VALUE},
                {{}, "value", "value", valueUnionSpec, lyric_parser::BindingType::VALUE}
            },
            {},
            lyric_object::AccessType::Public,
            static_cast<uint32_t>(StdSystemTrap::APPEND_OPERATION_ALLOC));
        auto *call = cast_symbol_to_call(symbolCache->getOrImportSymbol(declareCtorResult.getResult()).orElseThrow());
        auto *code = call->callProc()->procCode();
        // call the super constructor
        code->loadSynthetic(lyric_assembler::SyntheticType::THIS);
        code->callVirtual(superCtorCall->getAddress(), 0);
        // load the path argument and store it in member
        code->loadArgument(lyric_assembler::ArgumentOffset(0));
        code->storeField(PathField->getAddress());
        // load the value argument and store it in member
        code->loadArgument(lyric_assembler::ArgumentOffset(1));
        code->storeField(ValueField->getAddress());
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareFunctionResult = block->declareFunction("AppendOperation.$create",
            {
                {{}, "path", "", StringSpec, lyric_parser::BindingType::VALUE},
                {{}, "value", "", valueUnionSpec, lyric_parser::BindingType::VALUE},
            },
            {},
            {},
            lyric_parser::Assignable::forSingular({"AppendOperation"}),
            lyric_object::AccessType::Public,
            {});
        if (declareFunctionResult.isStatus())
            return declareFunctionResult.getStatus();
        auto functionUrl = declareFunctionResult.getResult();
        auto *call = cast_symbol_to_call(symbolCache->getOrImportSymbol(functionUrl).orElseThrow());
        auto *proc = call->callProc();
        auto *code = proc->procCode();
        auto *createBlock = proc->procBlock();

        auto resolveCtorResult = AppendOperationStruct->resolveCtor();
        if (resolveCtorResult.isStatus())
            return resolveCtorResult.getStatus();
        auto ctor = resolveCtorResult.getResult();
        lyric_typing::CallsiteReifier ctorReifier(ctor.getParameters(), ctor.getRest(),
            ctor.getTemplateUrl(), ctor.getTemplateParameters(), {}, typeSystem);
        TU_RETURN_IF_NOT_OK (ctorReifier.initialize());

        TU_RETURN_IF_NOT_OK (code->loadArgument(lyric_assembler::ArgumentOffset(0)));
        TU_RETURN_IF_NOT_OK (ctorReifier.reifyNextArgument(call->getParameters().at(0).typeDef));
        TU_RETURN_IF_NOT_OK (code->loadArgument(lyric_assembler::ArgumentOffset(1)));
        TU_RETURN_IF_NOT_OK (ctorReifier.reifyNextArgument(call->getParameters().at(1).typeDef));
        auto invokeNewResult = ctor.invokeNew(createBlock, ctorReifier);
        if (invokeNewResult.isStatus())
            return invokeNewResult.getStatus();
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return lyric_assembler::AssemblerStatus::ok();
}

tempo_utils::Status
build_std_system_InsertOperation(
    lyric_assembler::StructSymbol *OperationStruct,
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem)
{
    auto *symbolCache = state.symbolCache();

    auto declareInsertOperationStructResult = block->declareStruct("InsertOperation",
        OperationStruct, lyric_object::AccessType::Public, lyric_object::DeriveType::Final);
    if (declareInsertOperationStructResult.isStatus())
        return declareInsertOperationStructResult.getStatus();
    auto *InsertOperationStruct = cast_symbol_to_struct(
        symbolCache->getOrImportSymbol(declareInsertOperationStructResult.getResult()).orElseThrow());

    //
    OperationStruct->putSealedType(InsertOperationStruct->getAssignableType());

    auto StringSpec = lyric_parser::Assignable::forSingular({"String"});
    auto IntSpec = lyric_parser::Assignable::forSingular({"Int"});
    auto valueUnionSpec = lyric_parser::Assignable::forUnion({
            lyric_parser::Assignable::forSingular({"Intrinsic"}),
            lyric_parser::Assignable::forSingular({"Attr"}),
            lyric_parser::Assignable::forSingular({"Element"}),
        });

    // declare the "path" member
    auto declarePathMemberResult = InsertOperationStruct->declareMember("path", StringSpec);
    if (declarePathMemberResult.isStatus())
        return declarePathMemberResult.getStatus();
    auto *PathField = cast_symbol_to_field(
        symbolCache->getOrImportSymbol(declarePathMemberResult.getResult().symbolUrl).orElseThrow());

    // declare the "index" member
    auto declareIndexMemberResult = InsertOperationStruct->declareMember("index", IntSpec);
    if (declareIndexMemberResult.isStatus())
        return declareIndexMemberResult.getStatus();
    auto *IndexField = cast_symbol_to_field(
        symbolCache->getOrImportSymbol(declareIndexMemberResult.getResult().symbolUrl).orElseThrow());

    // declare the "value" member
    auto declareValueMemberResult = InsertOperationStruct->declareMember("value", valueUnionSpec);
    if (declareValueMemberResult.isStatus())
        return declareValueMemberResult.getStatus();
    auto *ValueField = cast_symbol_to_field(
        symbolCache->getOrImportSymbol(declareValueMemberResult.getResult().symbolUrl).orElseThrow());

    {
        auto superCtorUrl = OperationStruct->getCtor();
        if (!symbolCache->hasSymbol(superCtorUrl))
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "missing ctor for InsertOperation");
        lyric_assembler::AbstractSymbol *superCtorSym;
        TU_ASSIGN_OR_RETURN (superCtorSym, symbolCache->getOrImportSymbol(superCtorUrl));
        if (superCtorSym->getSymbolType() != lyric_assembler::SymbolType::CALL)
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid ctor for InsertOperation");
        auto *superCtorCall = cast_symbol_to_call(superCtorSym);

        auto declareCtorResult = InsertOperationStruct->declareCtor(
            {
                {{}, "path", "path", StringSpec, lyric_parser::BindingType::VALUE},
                {{}, "index", "index", IntSpec, lyric_parser::BindingType::VALUE},
                {{}, "value", "value", valueUnionSpec, lyric_parser::BindingType::VALUE}
            },
            {},
            lyric_object::AccessType::Public,
            static_cast<uint32_t>(StdSystemTrap::INSERT_OPERATION_ALLOC));
        auto *call = cast_symbol_to_call(symbolCache->getOrImportSymbol(declareCtorResult.getResult()).orElseThrow());
        auto *code = call->callProc()->procCode();
        // call the super constructor
        code->loadSynthetic(lyric_assembler::SyntheticType::THIS);
        code->callVirtual(superCtorCall->getAddress(), 0);
        // load the path argument and store it in member
        code->loadArgument(lyric_assembler::ArgumentOffset(0));
        code->storeField(PathField->getAddress());
        // load the index argument and store it in member
        code->loadArgument(lyric_assembler::ArgumentOffset(1));
        code->storeField(IndexField->getAddress());
        // load the value argument and store it in member
        code->loadArgument(lyric_assembler::ArgumentOffset(2));
        code->storeField(ValueField->getAddress());
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareFunctionResult = block->declareFunction("InsertOperation.$create",
            {
                {{}, "path", "", StringSpec, lyric_parser::BindingType::VALUE},
                {{}, "index", "", IntSpec, lyric_parser::BindingType::VALUE},
                {{}, "value", "", valueUnionSpec, lyric_parser::BindingType::VALUE},
            },
            {},
            {},
            lyric_parser::Assignable::forSingular({"InsertOperation"}),
            lyric_object::AccessType::Public,
            {});
        if (declareFunctionResult.isStatus())
            return declareFunctionResult.getStatus();
        auto functionUrl = declareFunctionResult.getResult();
        auto *call = cast_symbol_to_call(symbolCache->getOrImportSymbol(functionUrl).orElseThrow());
        auto *proc = call->callProc();
        auto *code = proc->procCode();
        auto *createBlock = proc->procBlock();

        auto resolveCtorResult = InsertOperationStruct->resolveCtor();
        if (resolveCtorResult.isStatus())
            return resolveCtorResult.getStatus();
        auto ctor = resolveCtorResult.getResult();
        lyric_typing::CallsiteReifier ctorReifier(ctor.getParameters(), ctor.getRest(),
            ctor.getTemplateUrl(), ctor.getTemplateParameters(), {}, typeSystem);
        TU_RETURN_IF_NOT_OK (ctorReifier.initialize());

        TU_RETURN_IF_NOT_OK (code->loadArgument(lyric_assembler::ArgumentOffset(0)));
        TU_RETURN_IF_NOT_OK (ctorReifier.reifyNextArgument(call->getParameters().at(0).typeDef));
        TU_RETURN_IF_NOT_OK (code->loadArgument(lyric_assembler::ArgumentOffset(1)));
        TU_RETURN_IF_NOT_OK (ctorReifier.reifyNextArgument(call->getParameters().at(1).typeDef));
        TU_RETURN_IF_NOT_OK (code->loadArgument(lyric_assembler::ArgumentOffset(2)));
        TU_RETURN_IF_NOT_OK (ctorReifier.reifyNextArgument(call->getParameters().at(2).typeDef));
        auto invokeNewResult = ctor.invokeNew(createBlock, ctorReifier);
        if (invokeNewResult.isStatus())
            return invokeNewResult.getStatus();
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return lyric_assembler::AssemblerStatus::ok();
}

tempo_utils::Status
build_std_system_UpdateOperation(
    lyric_assembler::StructSymbol *OperationStruct,
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem)
{
    auto *symbolCache = state.symbolCache();

    auto declareInsertOperationStructResult = block->declareStruct("UpdateOperation",
        OperationStruct, lyric_object::AccessType::Public, lyric_object::DeriveType::Final);
    if (declareInsertOperationStructResult.isStatus())
        return declareInsertOperationStructResult.getStatus();
    auto *InsertOperationStruct = cast_symbol_to_struct(
        symbolCache->getOrImportSymbol(declareInsertOperationStructResult.getResult()).orElseThrow());

    //
    OperationStruct->putSealedType(InsertOperationStruct->getAssignableType());

    auto StringSpec = lyric_parser::Assignable::forSingular({"String"});
    auto UrlSpec = lyric_parser::Assignable::forSingular({"Url"});
    auto IntSpec = lyric_parser::Assignable::forSingular({"Int"});
    auto valueUnionSpec = lyric_parser::Assignable::forUnion({
            lyric_parser::Assignable::forSingular({"Intrinsic"}),
            lyric_parser::Assignable::forSingular({"Attr"}),
            lyric_parser::Assignable::forSingular({"Element"}),
        });

    // declare the "path" member
    auto declarePathMemberResult = InsertOperationStruct->declareMember("path", StringSpec);
    if (declarePathMemberResult.isStatus())
        return declarePathMemberResult.getStatus();
    auto *PathField = cast_symbol_to_field(
        symbolCache->getOrImportSymbol(declarePathMemberResult.getResult().symbolUrl).orElseThrow());

    // declare the "ns" member
    auto declareNsMemberResult = InsertOperationStruct->declareMember("ns", UrlSpec);
    if (declareNsMemberResult.isStatus())
        return declareNsMemberResult.getStatus();
    auto *NsField = cast_symbol_to_field(
        symbolCache->getOrImportSymbol(declareNsMemberResult.getResult().symbolUrl).orElseThrow());

    // declare the "id" member
    auto declareIdMemberResult = InsertOperationStruct->declareMember("id", IntSpec);
    if (declareIdMemberResult.isStatus())
        return declareIdMemberResult.getStatus();
    auto *IdField = cast_symbol_to_field(
        symbolCache->getOrImportSymbol(declareIdMemberResult.getResult().symbolUrl).orElseThrow());

    // declare the "value" member
    auto declareValueMemberResult = InsertOperationStruct->declareMember("value", valueUnionSpec);
    if (declareValueMemberResult.isStatus())
        return declareValueMemberResult.getStatus();
    auto *ValueField = cast_symbol_to_field(
        symbolCache->getOrImportSymbol(declareValueMemberResult.getResult().symbolUrl).orElseThrow());

    {
        auto superCtorUrl = OperationStruct->getCtor();
        if (!symbolCache->hasSymbol(superCtorUrl))
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "missing ctor for UpdateOperation");
        lyric_assembler::AbstractSymbol *superCtorSym;
        TU_ASSIGN_OR_RETURN (superCtorSym, symbolCache->getOrImportSymbol(superCtorUrl));
        if (superCtorSym->getSymbolType() != lyric_assembler::SymbolType::CALL)
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid ctor for UpdateOperation");
        auto *superCtorCall = cast_symbol_to_call(superCtorSym);

        auto declareCtorResult = InsertOperationStruct->declareCtor(
            {
                {{}, "path", "path", StringSpec, lyric_parser::BindingType::VALUE},
                {{}, "ns", "ns", UrlSpec, lyric_parser::BindingType::VALUE},
                {{}, "id", "id", IntSpec, lyric_parser::BindingType::VALUE},
                {{}, "value", "value", valueUnionSpec, lyric_parser::BindingType::VALUE}
            },
            {},
            lyric_object::AccessType::Public,
            static_cast<uint32_t>(StdSystemTrap::UPDATE_OPERATION_ALLOC));
        auto *call = cast_symbol_to_call(symbolCache->getOrImportSymbol(declareCtorResult.getResult()).orElseThrow());
        auto *code = call->callProc()->procCode();
        // call the super constructor
        code->loadSynthetic(lyric_assembler::SyntheticType::THIS);
        code->callVirtual(superCtorCall->getAddress(), 0);
        // load the path argument and store it in member
        code->loadArgument(lyric_assembler::ArgumentOffset(0));
        code->storeField(PathField->getAddress());
        // load the ns argument and store it in member
        code->loadArgument(lyric_assembler::ArgumentOffset(1));
        code->storeField(NsField->getAddress());
        // load the id argument and store it in member
        code->loadArgument(lyric_assembler::ArgumentOffset(2));
        code->storeField(IdField->getAddress());
        // load the value argument and store it in member
        code->loadArgument(lyric_assembler::ArgumentOffset(3));
        code->storeField(ValueField->getAddress());
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareFunctionResult = block->declareFunction("UpdateOperation.$create",
            {
                {{}, "path", "", StringSpec, lyric_parser::BindingType::VALUE},
                {{}, "ns", "", UrlSpec, lyric_parser::BindingType::VALUE},
                {{}, "id", "", IntSpec, lyric_parser::BindingType::VALUE},
                {{}, "value", "", valueUnionSpec, lyric_parser::BindingType::VALUE},
            },
            {},
            {},
            lyric_parser::Assignable::forSingular({"UpdateOperation"}),
            lyric_object::AccessType::Public,
            {});
        if (declareFunctionResult.isStatus())
            return declareFunctionResult.getStatus();
        auto functionUrl = declareFunctionResult.getResult();
        auto *call = cast_symbol_to_call(symbolCache->getOrImportSymbol(functionUrl).orElseThrow());
        auto *proc = call->callProc();
        auto *code = proc->procCode();
        auto *createBlock = proc->procBlock();

        auto resolveCtorResult = InsertOperationStruct->resolveCtor();
        if (resolveCtorResult.isStatus())
            return resolveCtorResult.getStatus();
        auto ctor = resolveCtorResult.getResult();
        lyric_typing::CallsiteReifier ctorReifier(ctor.getParameters(), ctor.getRest(),
            ctor.getTemplateUrl(), ctor.getTemplateParameters(), {}, typeSystem);
        TU_RETURN_IF_NOT_OK (ctorReifier.initialize());

        lyric_assembler::AssemblerStatus status;
        TU_RETURN_IF_NOT_OK (code->loadArgument(lyric_assembler::ArgumentOffset(0)));
        TU_RETURN_IF_NOT_OK (ctorReifier.reifyNextArgument(call->getParameters().at(0).typeDef));
        TU_RETURN_IF_NOT_OK (code->loadArgument(lyric_assembler::ArgumentOffset(1)));
        TU_RETURN_IF_NOT_OK (ctorReifier.reifyNextArgument(call->getParameters().at(1).typeDef));
        TU_RETURN_IF_NOT_OK (code->loadArgument(lyric_assembler::ArgumentOffset(2)));
        TU_RETURN_IF_NOT_OK (ctorReifier.reifyNextArgument(call->getParameters().at(2).typeDef));
        TU_RETURN_IF_NOT_OK (code->loadArgument(lyric_assembler::ArgumentOffset(3)));
        TU_RETURN_IF_NOT_OK (ctorReifier.reifyNextArgument(call->getParameters().at(3).typeDef));
        auto invokeNewResult = ctor.invokeNew(createBlock, ctorReifier);
        if (invokeNewResult.isStatus())
            return invokeNewResult.getStatus();
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return lyric_assembler::AssemblerStatus::ok();
}

tempo_utils::Status
build_std_system_ReplaceOperation(
    lyric_assembler::StructSymbol *OperationStruct,
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem)
{
    auto *symbolCache = state.symbolCache();

    auto declareReplaceOperationStructResult = block->declareStruct("ReplaceOperation",
        OperationStruct, lyric_object::AccessType::Public, lyric_object::DeriveType::Final);
    if (declareReplaceOperationStructResult.isStatus())
        return declareReplaceOperationStructResult.getStatus();
    auto *ReplaceOperationStruct = cast_symbol_to_struct(
        symbolCache->getOrImportSymbol(declareReplaceOperationStructResult.getResult()).orElseThrow());

    //
    OperationStruct->putSealedType(ReplaceOperationStruct->getAssignableType());

    auto StringSpec = lyric_parser::Assignable::forSingular({"String"});
    auto valueUnionSpec = lyric_parser::Assignable::forUnion({
            lyric_parser::Assignable::forSingular({"Intrinsic"}),
            lyric_parser::Assignable::forSingular({"Attr"}),
            lyric_parser::Assignable::forSingular({"Element"}),
        });

    // declare the "path" member
    auto declarePathMemberResult = ReplaceOperationStruct->declareMember("path", StringSpec);
    if (declarePathMemberResult.isStatus())
        return declarePathMemberResult.getStatus();
    auto *PathField = cast_symbol_to_field(
        symbolCache->getOrImportSymbol(declarePathMemberResult.getResult().symbolUrl).orElseThrow());

    // declare the "value" member
    auto declareValueMemberResult = ReplaceOperationStruct->declareMember("value", valueUnionSpec);
    if (declareValueMemberResult.isStatus())
        return declareValueMemberResult.getStatus();
    auto *ValueField = cast_symbol_to_field(
        symbolCache->getOrImportSymbol(declareValueMemberResult.getResult().symbolUrl).orElseThrow());

    {
        auto superCtorUrl = OperationStruct->getCtor();
        if (!symbolCache->hasSymbol(superCtorUrl))
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "missing ctor for ReplaceOperation");
        lyric_assembler::AbstractSymbol *superCtorSym;
        TU_ASSIGN_OR_RETURN (superCtorSym, symbolCache->getOrImportSymbol(superCtorUrl));
        if (superCtorSym->getSymbolType() != lyric_assembler::SymbolType::CALL)
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid ctor for ReplaceOperation");
        auto *superCtorCall = cast_symbol_to_call(superCtorSym);

        auto declareCtorResult = ReplaceOperationStruct->declareCtor(
            {
                {{}, "path", "path", StringSpec, lyric_parser::BindingType::VALUE},
                {{}, "value", "value", valueUnionSpec, lyric_parser::BindingType::VALUE}
            },
            {},
            lyric_object::AccessType::Public,
            static_cast<uint32_t>(StdSystemTrap::REPLACE_OPERATION_ALLOC));
        auto *call = cast_symbol_to_call(symbolCache->getOrImportSymbol(declareCtorResult.getResult()).orElseThrow());
        auto *code = call->callProc()->procCode();
        // call the super constructor
        code->loadSynthetic(lyric_assembler::SyntheticType::THIS);
        code->callVirtual(superCtorCall->getAddress(), 0);
        // load the path argument and store it in member
        code->loadArgument(lyric_assembler::ArgumentOffset(0));
        code->storeField(PathField->getAddress());
        // load the value argument and store it in member
        code->loadArgument(lyric_assembler::ArgumentOffset(1));
        code->storeField(ValueField->getAddress());
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareFunctionResult = block->declareFunction("ReplaceOperation.$create",
            {
                {{}, "path", "", StringSpec, lyric_parser::BindingType::VALUE},
                {{}, "value", "", valueUnionSpec, lyric_parser::BindingType::VALUE},
            },
            {},
            {},
            lyric_parser::Assignable::forSingular({"ReplaceOperation"}),
            lyric_object::AccessType::Public,
            {});
        if (declareFunctionResult.isStatus())
            return declareFunctionResult.getStatus();
        auto functionUrl = declareFunctionResult.getResult();
        auto *call = cast_symbol_to_call(symbolCache->getOrImportSymbol(functionUrl).orElseThrow());
        auto *proc = call->callProc();
        auto *code = proc->procCode();
        auto *createBlock = proc->procBlock();

        auto resolveCtorResult = ReplaceOperationStruct->resolveCtor();
        if (resolveCtorResult.isStatus())
            return resolveCtorResult.getStatus();
        auto ctor = resolveCtorResult.getResult();
        lyric_typing::CallsiteReifier ctorReifier(ctor.getParameters(), ctor.getRest(),
            ctor.getTemplateUrl(), ctor.getTemplateParameters(), {}, typeSystem);
        TU_RETURN_IF_NOT_OK (ctorReifier.initialize());

        TU_RETURN_IF_NOT_OK (code->loadArgument(lyric_assembler::ArgumentOffset(0)));
        TU_RETURN_IF_NOT_OK (ctorReifier.reifyNextArgument(call->getParameters().at(0).typeDef));
        TU_RETURN_IF_NOT_OK (code->loadArgument(lyric_assembler::ArgumentOffset(1)));
        TU_RETURN_IF_NOT_OK (ctorReifier.reifyNextArgument(call->getParameters().at(1).typeDef));
        auto invokeNewResult = ctor.invokeNew(createBlock, ctorReifier);
        if (invokeNewResult.isStatus())
            return invokeNewResult.getStatus();
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return lyric_assembler::AssemblerStatus::ok();
}

tempo_utils::Status
build_std_system_EmitOperation(
    lyric_assembler::StructSymbol *OperationStruct,
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem)
{
    auto *symbolCache = state.symbolCache();

    auto declareEmitOperationStructResult = block->declareStruct("EmitOperation",
        OperationStruct, lyric_object::AccessType::Public, lyric_object::DeriveType::Final);
    if (declareEmitOperationStructResult.isStatus())
        return declareEmitOperationStructResult.getStatus();
    auto *EmitOperationStruct = cast_symbol_to_struct(
        symbolCache->getOrImportSymbol(declareEmitOperationStructResult.getResult()).orElseThrow());

    //
    OperationStruct->putSealedType(EmitOperationStruct->getAssignableType());

    auto valueUnionSpec = lyric_parser::Assignable::forUnion({
        lyric_parser::Assignable::forSingular({"Intrinsic"}),
        lyric_parser::Assignable::forSingular({"Attr"}),
        lyric_parser::Assignable::forSingular({"Element"}),
    });

    // declare the "value" member
    auto declareValueMemberResult = EmitOperationStruct->declareMember("value", valueUnionSpec);
    if (declareValueMemberResult.isStatus())
        return declareValueMemberResult.getStatus();
    auto *IdField = cast_symbol_to_field(
        symbolCache->getOrImportSymbol(declareValueMemberResult.getResult().symbolUrl).orElseThrow());

    {
        auto superCtorUrl = OperationStruct->getCtor();
        if (!symbolCache->hasSymbol(superCtorUrl))
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "missing ctor for EmitOperation");
        lyric_assembler::AbstractSymbol *superCtorSym;
        TU_ASSIGN_OR_RETURN (superCtorSym, symbolCache->getOrImportSymbol(superCtorUrl));
        if (superCtorSym->getSymbolType() != lyric_assembler::SymbolType::CALL)
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid ctor for EmitOperation");
        auto *superCtorCall = cast_symbol_to_call(superCtorSym);

        auto declareCtorResult = EmitOperationStruct->declareCtor(
            {
                {{}, "value", "value", valueUnionSpec, lyric_parser::BindingType::VALUE}
            },
            {},
            lyric_object::AccessType::Public,
            static_cast<uint32_t>(StdSystemTrap::EMIT_OPERATION_ALLOC));
        auto *call = cast_symbol_to_call(symbolCache->getOrImportSymbol(declareCtorResult.getResult()).orElseThrow());
        auto *code = call->callProc()->procCode();
        // call the super constructor
        code->loadSynthetic(lyric_assembler::SyntheticType::THIS);
        code->callVirtual(superCtorCall->getAddress(), 0);
        // load the value argument and store it in member
        code->loadArgument(lyric_assembler::ArgumentOffset(0));
        code->storeField(IdField->getAddress());
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareFunctionResult = block->declareFunction("EmitOperation.$create",
            {
                {{}, "value", "", valueUnionSpec, lyric_parser::BindingType::VALUE},
            },
            {},
            {},
            lyric_parser::Assignable::forSingular({"EmitOperation"}),
            lyric_object::AccessType::Public,
            {});
        if (declareFunctionResult.isStatus())
            return declareFunctionResult.getStatus();
        auto functionUrl = declareFunctionResult.getResult();
        auto *call = cast_symbol_to_call(symbolCache->getOrImportSymbol(functionUrl).orElseThrow());
        auto *proc = call->callProc();
        auto *code = proc->procCode();
        auto *createBlock = proc->procBlock();

        auto resolveCtorResult = EmitOperationStruct->resolveCtor();
        if (resolveCtorResult.isStatus())
            return resolveCtorResult.getStatus();
        auto ctor = resolveCtorResult.getResult();
        lyric_typing::CallsiteReifier ctorReifier(ctor.getParameters(), ctor.getRest(),
            ctor.getTemplateUrl(), ctor.getTemplateParameters(), {}, typeSystem);
        TU_RETURN_IF_NOT_OK (ctorReifier.initialize());

        code->loadArgument(lyric_assembler::ArgumentOffset(0));
        auto status = ctorReifier.reifyNextArgument(call->getParameters().front().typeDef);
        if (status.notOk())
            return status;
        auto invokeNewResult = ctor.invokeNew(createBlock, ctorReifier);
        if (invokeNewResult.isStatus())
            return invokeNewResult.getStatus();
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return lyric_assembler::AssemblerStatus::ok();
}
