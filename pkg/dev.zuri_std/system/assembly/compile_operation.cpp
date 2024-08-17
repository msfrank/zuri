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
    lyric_assembler::ObjectState &state,
    lyric_assembler::BlockHandle *block,
    lyric_assembler::StructSymbol *AttrStruct,
    lyric_typing::TypeSystem *typeSystem)
{
    auto *fundamentalCache = state.fundamentalCache();

    lyric_assembler::StructSymbol *RecordStruct;
    TU_ASSIGN_OR_RETURN (RecordStruct, block->resolveStruct(
        fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Record)));

    lyric_assembler::StructSymbol *OperationStruct;
    TU_ASSIGN_OR_RETURN (OperationStruct, block->declareStruct(
        "Operation", RecordStruct, lyric_object::AccessType::Public, lyric_object::DeriveType::Sealed, true));

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
    lyric_assembler::ObjectState &state,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem)
{
    auto *fundamentalCache = state.fundamentalCache();
    auto *symbolCache = state.symbolCache();

    lyric_assembler::StructSymbol *AppendOperationStruct;
    TU_ASSIGN_OR_RETURN (AppendOperationStruct, block->declareStruct(
        "AppendOperation", OperationStruct, lyric_object::AccessType::Public, lyric_object::DeriveType::Final));

    //
    OperationStruct->putSealedType(AppendOperationStruct->getAssignableType());

    auto StringType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::String);
    auto IntrinsicType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Intrinsic);
    auto AttrType = lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl::fromString("#Attr"));
    auto ElementType = lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl::fromString("#Element"));
    auto ValueType = lyric_common::TypeDef::forUnion({ IntrinsicType, AttrType, ElementType });

    // declare the "path" member
    lyric_assembler::FieldSymbol *PathField;
    TU_ASSIGN_OR_RETURN (PathField, AppendOperationStruct->declareMember("path", StringType));

    // declare the "value" member
    lyric_assembler::FieldSymbol *ValueField;
    TU_ASSIGN_OR_RETURN (ValueField, AppendOperationStruct->declareMember("value", ValueType));

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
    lyric_assembler::ObjectState &state,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem)
{
    auto *fundamentalCache = state.fundamentalCache();
    auto *symbolCache = state.symbolCache();

    lyric_assembler::StructSymbol *InsertOperationStruct;
    TU_ASSIGN_OR_RETURN (InsertOperationStruct, block->declareStruct(
        "InsertOperation", OperationStruct, lyric_object::AccessType::Public, lyric_object::DeriveType::Final));

    //
    OperationStruct->putSealedType(InsertOperationStruct->getAssignableType());

    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    auto StringType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::String);
    auto IntrinsicType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Intrinsic);
    auto AttrType = lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl::fromString("#Attr"));
    auto ElementType = lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl::fromString("#Element"));
    auto ValueType = lyric_common::TypeDef::forUnion({ IntrinsicType, AttrType, ElementType });

    // declare the "path" member
    lyric_assembler::FieldSymbol *PathField;
    TU_ASSIGN_OR_RETURN (PathField, InsertOperationStruct->declareMember("path", StringType));

    // declare the "index" member
    lyric_assembler::FieldSymbol *IndexField;
    TU_ASSIGN_OR_RETURN (IndexField, InsertOperationStruct->declareMember("index", IntType));

    // declare the "value" member
    lyric_assembler::FieldSymbol *ValueField;
    TU_ASSIGN_OR_RETURN (ValueField, InsertOperationStruct->declareMember("value", ValueType));

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
    lyric_assembler::ObjectState &state,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem)
{
    auto *fundamentalCache = state.fundamentalCache();
    auto *symbolCache = state.symbolCache();

    lyric_assembler::StructSymbol *UpdateOperationStruct;
    TU_ASSIGN_OR_RETURN (UpdateOperationStruct, block->declareStruct(
        "UpdateOperation", OperationStruct, lyric_object::AccessType::Public, lyric_object::DeriveType::Final));

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
    lyric_assembler::FieldSymbol *PathField;
    TU_ASSIGN_OR_RETURN (PathField, UpdateOperationStruct->declareMember("path", StringType));

    // declare the "ns" member
    lyric_assembler::FieldSymbol *NsField;
    TU_ASSIGN_OR_RETURN (NsField, UpdateOperationStruct->declareMember("ns", UrlType));

    // declare the "id" member
    lyric_assembler::FieldSymbol *IdField;
    TU_ASSIGN_OR_RETURN (IdField, UpdateOperationStruct->declareMember("id", IntType));

    // declare the "value" member
    lyric_assembler::FieldSymbol *ValueField;
    TU_ASSIGN_OR_RETURN (ValueField, UpdateOperationStruct->declareMember("value", ValueType));

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
    lyric_assembler::ObjectState &state,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem)
{
    auto *fundamentalCache = state.fundamentalCache();
    auto *symbolCache = state.symbolCache();

    lyric_assembler::StructSymbol *ReplaceOperationStruct;
    TU_ASSIGN_OR_RETURN (ReplaceOperationStruct, block->declareStruct(
        "ReplaceOperation", OperationStruct, lyric_object::AccessType::Public, lyric_object::DeriveType::Final));

    //
    OperationStruct->putSealedType(ReplaceOperationStruct->getAssignableType());

    auto StringType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::String);
    auto IntrinsicType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Intrinsic);
    auto AttrType = lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl::fromString("#Attr"));
    auto ElementType = lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl::fromString("#Element"));
    auto ValueType = lyric_common::TypeDef::forUnion({ IntrinsicType, AttrType, ElementType });

    // declare the "path" member
    lyric_assembler::FieldSymbol *PathField;
    TU_ASSIGN_OR_RETURN (PathField, ReplaceOperationStruct->declareMember("path", StringType));

    // declare the "value" member
    lyric_assembler::FieldSymbol *ValueField;
    TU_ASSIGN_OR_RETURN (ValueField, ReplaceOperationStruct->declareMember("value", ValueType));

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
    lyric_assembler::ObjectState &state,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem)
{
    auto *fundamentalCache = state.fundamentalCache();
    auto *symbolCache = state.symbolCache();

    lyric_assembler::StructSymbol *EmitOperationStruct;
    TU_ASSIGN_OR_RETURN (EmitOperationStruct, block->declareStruct(
        "EmitOperation", OperationStruct, lyric_object::AccessType::Public, lyric_object::DeriveType::Final));

    //
    OperationStruct->putSealedType(EmitOperationStruct->getAssignableType());

    auto IntrinsicType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Intrinsic);
    auto AttrType = lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl::fromString("#Attr"));
    auto ElementType = lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl::fromString("#Element"));
    auto ValueType = lyric_common::TypeDef::forUnion({ IntrinsicType, AttrType, ElementType });

    // declare the "value" member
    lyric_assembler::FieldSymbol *ValueField;
    TU_ASSIGN_OR_RETURN (ValueField, EmitOperationStruct->declareMember("value", ValueType));

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
        codeBuilder->storeField(ValueField->getAddress());
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
