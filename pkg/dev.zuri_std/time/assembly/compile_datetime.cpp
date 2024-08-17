/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/pack_builder.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <zuri_std_time/lib_types.h>

#include "compile_datetime.h"

tempo_utils::Status
build_std_time_Datetime(
    lyric_assembler::ObjectState &state,
    lyric_assembler::BlockHandle *block)
{
    auto *fundamentalCache = state.fundamentalCache();

    lyric_assembler::ClassSymbol *ObjectClass;
    TU_ASSIGN_OR_RETURN (ObjectClass, block->resolveClass(
        fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object)));

    lyric_assembler::ClassSymbol *DatetimeClass;
    TU_ASSIGN_OR_RETURN (DatetimeClass, block->declareClass(
        "Datetime", ObjectClass, lyric_object::AccessType::Public, {}));

    auto InstantType = lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl::fromString("#Instant"));
    auto TimezoneType = lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl::fromString("#Timezone"));

    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, DatetimeClass->declareCtor(
            lyric_object::AccessType::Public, static_cast<tu_uint32>(StdTimeTrap::DATETIME_ALLOC)));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("instant", "", InstantType, false));
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("timezone", "", TimezoneType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, lyric_common::TypeDef::noReturn()));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdTimeTrap::DATETIME_CTOR));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return {};
}
