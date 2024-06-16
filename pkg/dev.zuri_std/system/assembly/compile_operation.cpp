/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/pack_builder.h>
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
    lyric_assembler::StructSymbol *AttrStruct,
    lyric_typing::TypeSystem *typeSystem)
{
    auto *fundamentalCache = state.fundamentalCache();
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

    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    auto StringType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::String);
    auto UrlType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Url);
    auto IntrinsicType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Intrinsic);
    auto AttrType = lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl::fromString("#Attr"));
    auto ElementType = lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl::fromString("#Element"));
    auto ValueType = lyric_common::TypeDef::forUnion({ IntrinsicType, ElementType });

    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, OperationStruct->declareCtor(lyric_object::AccessType::Public));
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall({}, lyric_common::TypeDef::noReturn()));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, block->declareFunction(
            "Operation.$createAttr", lyric_object::AccessType::Public, {}));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("ns", "", UrlType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("id", "", IntType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("value", "", ValueType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, AttrType));
        auto *codeBuilder = procHandle->procCode();
        auto *createBlock = procHandle->procBlock();

        lyric_assembler::ConstructableInvoker invoker;
        TU_RETURN_IF_NOT_OK (AttrStruct->prepareCtor(invoker));

        lyric_typing::CallsiteReifier ctorReifier(typeSystem);
        TU_RETURN_IF_NOT_OK (ctorReifier.initialize(invoker));

        TU_RETURN_IF_NOT_OK (codeBuilder->loadArgument(lyric_assembler::ArgumentOffset(0)));
        TU_RETURN_IF_NOT_OK (ctorReifier.reifyNextArgument(UrlType));
        TU_RETURN_IF_NOT_OK (codeBuilder->loadArgument(lyric_assembler::ArgumentOffset(1)));
        TU_RETURN_IF_NOT_OK (ctorReifier.reifyNextArgument(IntType));
        TU_RETURN_IF_NOT_OK (codeBuilder->loadArgument(lyric_assembler::ArgumentOffset(2)));
        TU_RETURN_IF_NOT_OK (ctorReifier.reifyNextArgument(ValueType));
        TU_RETURN_IF_STATUS (invoker.invokeNew(createBlock, ctorReifier, 0));
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
    auto *fundamentalCache = state.fundamentalCache();
    auto *symbolCache = state.symbolCache();

    auto declareAppendOperationStructResult = block->declareStruct("AppendOperation",
        OperationStruct, lyric_object::AccessType::Public, lyric_object::DeriveType::Final);
    if (declareAppendOperationStructResult.isStatus())
        return declareAppendOperationStructResult.getStatus();
    auto *AppendOperationStruct = cast_symbol_to_struct(
        symbolCache->getOrImportSymbol(declareAppendOperationStructResult.getResult()).orElseThrow());

    //
    OperationStruct->putSealedType(AppendOperationStruct->getAssignableType());

    auto StringType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::String);
    auto IntrinsicType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Intrinsic);
    auto AttrType = lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl::fromString("#Attr"));
    auto ElementType = lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl::fromString("#Element"));
    auto ValueType = lyric_common::TypeDef::forUnion({ IntrinsicType, AttrType, ElementType });

    // declare the "path" member
    auto declarePathMemberResult = AppendOperationStruct->declareMember("path", StringType);
    if (declarePathMemberResult.isStatus())
        return declarePathMemberResult.getStatus();
    auto *PathField = cast_symbol_to_field(
        symbolCache->getOrImportSymbol(declarePathMemberResult.getResult().symbolUrl).orElseThrow());

    // declare the "value" member
    auto declareValueMemberResult = AppendOperationStruct->declareMember("value", ValueType);
    if (declareValueMemberResult.isStatus())
        return declareValueMemberResult.getStatus();
    auto *ValueField = cast_symbol_to_field(
        symbolCache->getOrImportSymbol(declareValueMemberResult.getResult().symbolUrl).orElseThrow());

    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, AppendOperationStruct->declareCtor(
            lyric_object::AccessType::Public, static_cast<tu_uint32>(StdSystemTrap::APPEND_OPERATION_ALLOC)));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("path", "", StringType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("value", "", ValueType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, lyric_common::TypeDef::noReturn()));
        auto *codeBuilder = procHandle->procCode();

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

        // call the super constructor
        codeBuilder->loadSynthetic(lyric_assembler::SyntheticType::THIS);
        codeBuilder->callVirtual(superCtorCall->getAddress(), 0);
        // load the path argument and store it in member
        codeBuilder->loadArgument(lyric_assembler::ArgumentOffset(0));
        codeBuilder->storeField(PathField->getAddress());
        // load the value argument and store it in member
        codeBuilder->loadArgument(lyric_assembler::ArgumentOffset(1));
        codeBuilder->storeField(ValueField->getAddress());
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, block->declareFunction(
            "AppendOperation.$create", lyric_object::AccessType::Public, {}));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("path", "", StringType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("value", "", ValueType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(
            parameterPack, AppendOperationStruct->getAssignableType()));
        auto *codeBuilder = procHandle->procCode();
        auto *createBlock = procHandle->procBlock();

        lyric_assembler::ConstructableInvoker invoker;
        TU_RETURN_IF_NOT_OK (AppendOperationStruct->prepareCtor(invoker));
        lyric_typing::CallsiteReifier ctorReifier(typeSystem);
        TU_RETURN_IF_NOT_OK (ctorReifier.initialize(invoker));

        TU_RETURN_IF_NOT_OK (codeBuilder->loadArgument(lyric_assembler::ArgumentOffset(0)));
        TU_RETURN_IF_NOT_OK (ctorReifier.reifyNextArgument(StringType));
        TU_RETURN_IF_NOT_OK (codeBuilder->loadArgument(lyric_assembler::ArgumentOffset(1)));
        TU_RETURN_IF_NOT_OK (ctorReifier.reifyNextArgument(ValueType));
        TU_RETURN_IF_STATUS (invoker.invokeNew(createBlock, ctorReifier, 0));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return {};
}

tempo_utils::Status
build_std_system_InsertOperation(
    lyric_assembler::StructSymbol *OperationStruct,
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem)
{
    auto *fundamentalCache = state.fundamentalCache();
    auto *symbolCache = state.symbolCache();

    auto declareInsertOperationStructResult = block->declareStruct("InsertOperation",
        OperationStruct, lyric_object::AccessType::Public, lyric_object::DeriveType::Final);
    if (declareInsertOperationStructResult.isStatus())
        return declareInsertOperationStructResult.getStatus();
    auto *InsertOperationStruct = cast_symbol_to_struct(
        symbolCache->getOrImportSymbol(declareInsertOperationStructResult.getResult()).orElseThrow());

    //
    OperationStruct->putSealedType(InsertOperationStruct->getAssignableType());

    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    auto StringType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::String);
    auto IntrinsicType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Intrinsic);
    auto AttrType = lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl::fromString("#Attr"));
    auto ElementType = lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl::fromString("#Element"));
    auto ValueType = lyric_common::TypeDef::forUnion({ IntrinsicType, AttrType, ElementType });

    // declare the "path" member
    auto declarePathMemberResult = InsertOperationStruct->declareMember("path", StringType);
    if (declarePathMemberResult.isStatus())
        return declarePathMemberResult.getStatus();
    auto *PathField = cast_symbol_to_field(
        symbolCache->getOrImportSymbol(declarePathMemberResult.getResult().symbolUrl).orElseThrow());

    // declare the "index" member
    auto declareIndexMemberResult = InsertOperationStruct->declareMember("index", IntType);
    if (declareIndexMemberResult.isStatus())
        return declareIndexMemberResult.getStatus();
    auto *IndexField = cast_symbol_to_field(
        symbolCache->getOrImportSymbol(declareIndexMemberResult.getResult().symbolUrl).orElseThrow());

    // declare the "value" member
    auto declareValueMemberResult = InsertOperationStruct->declareMember("value", ValueType);
    if (declareValueMemberResult.isStatus())
        return declareValueMemberResult.getStatus();
    auto *ValueField = cast_symbol_to_field(
        symbolCache->getOrImportSymbol(declareValueMemberResult.getResult().symbolUrl).orElseThrow());

    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, InsertOperationStruct->declareCtor(
            lyric_object::AccessType::Public, static_cast<tu_uint32>(StdSystemTrap::INSERT_OPERATION_ALLOC)));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("path", "", StringType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("index", "", IntType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("value", "", ValueType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, lyric_common::TypeDef::noReturn()));
        auto *codeBuilder = procHandle->procCode();

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

        // call the super constructor
        codeBuilder->loadSynthetic(lyric_assembler::SyntheticType::THIS);
        codeBuilder->callVirtual(superCtorCall->getAddress(), 0);
        // load the path argument and store it in member
        codeBuilder->loadArgument(lyric_assembler::ArgumentOffset(0));
        codeBuilder->storeField(PathField->getAddress());
        // load the index argument and store it in member
        codeBuilder->loadArgument(lyric_assembler::ArgumentOffset(1));
        codeBuilder->storeField(IndexField->getAddress());
        // load the value argument and store it in member
        codeBuilder->loadArgument(lyric_assembler::ArgumentOffset(2));
        codeBuilder->storeField(ValueField->getAddress());
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, block->declareFunction(
            "InsertOperation.$create", lyric_object::AccessType::Public, {}));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("path", "", StringType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("index", "", IntType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("value", "", ValueType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(
            parameterPack, InsertOperationStruct->getAssignableType()));
        auto *codeBuilder = procHandle->procCode();
        auto *createBlock = procHandle->procBlock();

        lyric_assembler::ConstructableInvoker invoker;
        TU_RETURN_IF_NOT_OK (InsertOperationStruct->prepareCtor(invoker));
        lyric_typing::CallsiteReifier ctorReifier(typeSystem);
        TU_RETURN_IF_NOT_OK (ctorReifier.initialize(invoker));

        TU_RETURN_IF_NOT_OK (codeBuilder->loadArgument(lyric_assembler::ArgumentOffset(0)));
        TU_RETURN_IF_NOT_OK (ctorReifier.reifyNextArgument(StringType));
        TU_RETURN_IF_NOT_OK (codeBuilder->loadArgument(lyric_assembler::ArgumentOffset(1)));
        TU_RETURN_IF_NOT_OK (ctorReifier.reifyNextArgument(IntType));
        TU_RETURN_IF_NOT_OK (codeBuilder->loadArgument(lyric_assembler::ArgumentOffset(2)));
        TU_RETURN_IF_NOT_OK (ctorReifier.reifyNextArgument(ValueType));
        TU_RETURN_IF_STATUS (invoker.invokeNew(createBlock, ctorReifier, 0));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return {};
}

tempo_utils::Status
build_std_system_UpdateOperation(
    lyric_assembler::StructSymbol *OperationStruct,
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem)
{
    auto *fundamentalCache = state.fundamentalCache();
    auto *symbolCache = state.symbolCache();

    auto declareUpdateOperationStructResult = block->declareStruct("UpdateOperation",
        OperationStruct, lyric_object::AccessType::Public, lyric_object::DeriveType::Final);
    if (declareUpdateOperationStructResult.isStatus())
        return declareUpdateOperationStructResult.getStatus();
    auto *UpdateOperationStruct = cast_symbol_to_struct(
        symbolCache->getOrImportSymbol(declareUpdateOperationStructResult.getResult()).orElseThrow());

    //
    OperationStruct->putSealedType(UpdateOperationStruct->getAssignableType());

    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    auto StringType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::String);
    auto UrlType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Url);
    auto IntrinsicType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Intrinsic);
    auto AttrType = lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl::fromString("#Attr"));
    auto ElementType = lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl::fromString("#Element"));
    auto ValueType = lyric_common::TypeDef::forUnion({ IntrinsicType, AttrType, ElementType });

    // declare the "path" member
    auto declarePathMemberResult = UpdateOperationStruct->declareMember("path", StringType);
    if (declarePathMemberResult.isStatus())
        return declarePathMemberResult.getStatus();
    auto *PathField = cast_symbol_to_field(
        symbolCache->getOrImportSymbol(declarePathMemberResult.getResult().symbolUrl).orElseThrow());

    // declare the "ns" member
    auto declareNsMemberResult = UpdateOperationStruct->declareMember("ns", UrlType);
    if (declareNsMemberResult.isStatus())
        return declareNsMemberResult.getStatus();
    auto *NsField = cast_symbol_to_field(
        symbolCache->getOrImportSymbol(declareNsMemberResult.getResult().symbolUrl).orElseThrow());

    // declare the "id" member
    auto declareIdMemberResult = UpdateOperationStruct->declareMember("id", IntType);
    if (declareIdMemberResult.isStatus())
        return declareIdMemberResult.getStatus();
    auto *IdField = cast_symbol_to_field(
        symbolCache->getOrImportSymbol(declareIdMemberResult.getResult().symbolUrl).orElseThrow());

    // declare the "value" member
    auto declareValueMemberResult = UpdateOperationStruct->declareMember("value", ValueType);
    if (declareValueMemberResult.isStatus())
        return declareValueMemberResult.getStatus();
    auto *ValueField = cast_symbol_to_field(
        symbolCache->getOrImportSymbol(declareValueMemberResult.getResult().symbolUrl).orElseThrow());

    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, UpdateOperationStruct->declareCtor(
            lyric_object::AccessType::Public, static_cast<tu_uint32>(StdSystemTrap::UPDATE_OPERATION_ALLOC)));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("path", "", StringType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("ns", "", UrlType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("id", "", IntType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("value", "", ValueType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, lyric_common::TypeDef::noReturn()));
        auto *codeBuilder = procHandle->procCode();

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

        // call the super constructor
        codeBuilder->loadSynthetic(lyric_assembler::SyntheticType::THIS);
        codeBuilder->callVirtual(superCtorCall->getAddress(), 0);
        // load the path argument and store it in member
        codeBuilder->loadArgument(lyric_assembler::ArgumentOffset(0));
        codeBuilder->storeField(PathField->getAddress());
        // load the ns argument and store it in member
        codeBuilder->loadArgument(lyric_assembler::ArgumentOffset(1));
        codeBuilder->storeField(NsField->getAddress());
        // load the id argument and store it in member
        codeBuilder->loadArgument(lyric_assembler::ArgumentOffset(2));
        codeBuilder->storeField(IdField->getAddress());
        // load the value argument and store it in member
        codeBuilder->loadArgument(lyric_assembler::ArgumentOffset(3));
        codeBuilder->storeField(ValueField->getAddress());
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, block->declareFunction(
            "UpdateOperation.$create", lyric_object::AccessType::Public, {}));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("path", "", StringType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("ns", "", UrlType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("id", "", IntType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("value", "", ValueType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(
            parameterPack, UpdateOperationStruct->getAssignableType()));
        auto *codeBuilder = procHandle->procCode();
        auto *createBlock = procHandle->procBlock();

        lyric_assembler::ConstructableInvoker invoker;
        TU_RETURN_IF_NOT_OK (UpdateOperationStruct->prepareCtor(invoker));
        lyric_typing::CallsiteReifier ctorReifier(typeSystem);
        TU_RETURN_IF_NOT_OK (ctorReifier.initialize(invoker));

        lyric_assembler::AssemblerStatus status;
        TU_RETURN_IF_NOT_OK (codeBuilder->loadArgument(lyric_assembler::ArgumentOffset(0)));
        TU_RETURN_IF_NOT_OK (ctorReifier.reifyNextArgument(StringType));
        TU_RETURN_IF_NOT_OK (codeBuilder->loadArgument(lyric_assembler::ArgumentOffset(1)));
        TU_RETURN_IF_NOT_OK (ctorReifier.reifyNextArgument(UrlType));
        TU_RETURN_IF_NOT_OK (codeBuilder->loadArgument(lyric_assembler::ArgumentOffset(2)));
        TU_RETURN_IF_NOT_OK (ctorReifier.reifyNextArgument(IntType));
        TU_RETURN_IF_NOT_OK (codeBuilder->loadArgument(lyric_assembler::ArgumentOffset(3)));
        TU_RETURN_IF_NOT_OK (ctorReifier.reifyNextArgument(ValueType));
        TU_RETURN_IF_STATUS (invoker.invokeNew(createBlock, ctorReifier, 0));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return {};
}

tempo_utils::Status
build_std_system_ReplaceOperation(
    lyric_assembler::StructSymbol *OperationStruct,
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem)
{
    auto *fundamentalCache = state.fundamentalCache();
    auto *symbolCache = state.symbolCache();

    auto declareReplaceOperationStructResult = block->declareStruct("ReplaceOperation",
        OperationStruct, lyric_object::AccessType::Public, lyric_object::DeriveType::Final);
    if (declareReplaceOperationStructResult.isStatus())
        return declareReplaceOperationStructResult.getStatus();
    auto *ReplaceOperationStruct = cast_symbol_to_struct(
        symbolCache->getOrImportSymbol(declareReplaceOperationStructResult.getResult()).orElseThrow());

    //
    OperationStruct->putSealedType(ReplaceOperationStruct->getAssignableType());

    auto StringType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::String);
    auto IntrinsicType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Intrinsic);
    auto AttrType = lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl::fromString("#Attr"));
    auto ElementType = lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl::fromString("#Element"));
    auto ValueType = lyric_common::TypeDef::forUnion({ IntrinsicType, AttrType, ElementType });

    // declare the "path" member
    auto declarePathMemberResult = ReplaceOperationStruct->declareMember("path", StringType);
    if (declarePathMemberResult.isStatus())
        return declarePathMemberResult.getStatus();
    auto *PathField = cast_symbol_to_field(
        symbolCache->getOrImportSymbol(declarePathMemberResult.getResult().symbolUrl).orElseThrow());

    // declare the "value" member
    auto declareValueMemberResult = ReplaceOperationStruct->declareMember("value", ValueType);
    if (declareValueMemberResult.isStatus())
        return declareValueMemberResult.getStatus();
    auto *ValueField = cast_symbol_to_field(
        symbolCache->getOrImportSymbol(declareValueMemberResult.getResult().symbolUrl).orElseThrow());

    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, ReplaceOperationStruct->declareCtor(
            lyric_object::AccessType::Public, static_cast<tu_uint32>(StdSystemTrap::REPLACE_OPERATION_ALLOC)));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("path", "", StringType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("value", "", ValueType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, lyric_common::TypeDef::noReturn()));
        auto *codeBuilder = procHandle->procCode();

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

        // call the super constructor
        codeBuilder->loadSynthetic(lyric_assembler::SyntheticType::THIS);
        codeBuilder->callVirtual(superCtorCall->getAddress(), 0);
        // load the path argument and store it in member
        codeBuilder->loadArgument(lyric_assembler::ArgumentOffset(0));
        codeBuilder->storeField(PathField->getAddress());
        // load the value argument and store it in member
        codeBuilder->loadArgument(lyric_assembler::ArgumentOffset(1));
        codeBuilder->storeField(ValueField->getAddress());
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, block->declareFunction(
            "ReplaceOperation.$create", lyric_object::AccessType::Public, {}));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("path", "", StringType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("value", "", ValueType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(
            parameterPack, ReplaceOperationStruct->getAssignableType()));
        auto *codeBuilder = procHandle->procCode();
        auto *createBlock = procHandle->procBlock();

        lyric_assembler::ConstructableInvoker invoker;
        TU_RETURN_IF_NOT_OK (ReplaceOperationStruct->prepareCtor(invoker));
        lyric_typing::CallsiteReifier ctorReifier(typeSystem);
        TU_RETURN_IF_NOT_OK (ctorReifier.initialize(invoker));

        TU_RETURN_IF_NOT_OK (codeBuilder->loadArgument(lyric_assembler::ArgumentOffset(0)));
        TU_RETURN_IF_NOT_OK (ctorReifier.reifyNextArgument(StringType));
        TU_RETURN_IF_NOT_OK (codeBuilder->loadArgument(lyric_assembler::ArgumentOffset(1)));
        TU_RETURN_IF_NOT_OK (ctorReifier.reifyNextArgument(ValueType));
        TU_RETURN_IF_STATUS (invoker.invokeNew(createBlock, ctorReifier, 0));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
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
    auto *fundamentalCache = state.fundamentalCache();
    auto *symbolCache = state.symbolCache();

    auto declareEmitOperationStructResult = block->declareStruct("EmitOperation",
        OperationStruct, lyric_object::AccessType::Public, lyric_object::DeriveType::Final);
    if (declareEmitOperationStructResult.isStatus())
        return declareEmitOperationStructResult.getStatus();
    auto *EmitOperationStruct = cast_symbol_to_struct(
        symbolCache->getOrImportSymbol(declareEmitOperationStructResult.getResult()).orElseThrow());

    //
    OperationStruct->putSealedType(EmitOperationStruct->getAssignableType());

    auto IntrinsicType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Intrinsic);
    auto AttrType = lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl::fromString("#Attr"));
    auto ElementType = lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl::fromString("#Element"));
    auto ValueType = lyric_common::TypeDef::forUnion({ IntrinsicType, AttrType, ElementType });

    // declare the "value" member
    auto declareValueMemberResult = EmitOperationStruct->declareMember("value", ValueType);
    if (declareValueMemberResult.isStatus())
        return declareValueMemberResult.getStatus();
    auto *IdField = cast_symbol_to_field(
        symbolCache->getOrImportSymbol(declareValueMemberResult.getResult().symbolUrl).orElseThrow());

    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, EmitOperationStruct->declareCtor(
            lyric_object::AccessType::Public, static_cast<tu_uint32>(StdSystemTrap::EMIT_OPERATION_ALLOC)));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("value", "", ValueType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, lyric_common::TypeDef::noReturn()));
        auto *codeBuilder = procHandle->procCode();

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

        // call the super constructor
        codeBuilder->loadSynthetic(lyric_assembler::SyntheticType::THIS);
        codeBuilder->callVirtual(superCtorCall->getAddress(), 0);
        // load the value argument and store it in member
        codeBuilder->loadArgument(lyric_assembler::ArgumentOffset(0));
        codeBuilder->storeField(IdField->getAddress());
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, block->declareFunction(
            "EmitOperation.$create", lyric_object::AccessType::Public, {}));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("value", "", ValueType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(
            parameterPack, EmitOperationStruct->getAssignableType()));
        auto *codeBuilder = procHandle->procCode();
        auto *createBlock = procHandle->procBlock();

        lyric_assembler::ConstructableInvoker invoker;
        TU_RETURN_IF_NOT_OK (EmitOperationStruct->prepareCtor(invoker));
        lyric_typing::CallsiteReifier ctorReifier(typeSystem);
        TU_RETURN_IF_NOT_OK (ctorReifier.initialize(invoker));

        TU_RETURN_IF_NOT_OK (codeBuilder->loadArgument(lyric_assembler::ArgumentOffset(0)));
        TU_RETURN_IF_NOT_OK (ctorReifier.reifyNextArgument(ValueType));
        TU_RETURN_IF_STATUS (invoker.invokeNew(createBlock, ctorReifier, 0));
        TU_RETURN_IF_NOT_OK (codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN));
    }

    return {};
}
