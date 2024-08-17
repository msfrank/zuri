/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/pack_builder.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <zuri_std_time/lib_types.h>

#include "compile_timezone.h"

tempo_utils::Status
build_std_time_Timezone(
    lyric_assembler::ObjectState &state,
    lyric_assembler::BlockHandle *block)
{
    auto *fundamentalCache = state.fundamentalCache();

    lyric_assembler::ClassSymbol *ObjectClass;
    TU_ASSIGN_OR_RETURN (ObjectClass, block->resolveClass(
        fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object)));

    lyric_assembler::ClassSymbol *TimezoneClass;
    TU_ASSIGN_OR_RETURN (TimezoneClass, block->declareClass(
        "Timezone", ObjectClass, lyric_object::AccessType::Public, {}));

    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);

    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, TimezoneClass->declareCtor(
            lyric_object::AccessType::Public, static_cast<tu_uint32>(StdTimeTrap::TIMEZONE_ALLOC)));
        lyric_assembler::PackBuilder packBuilder;
        TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter("offset", "", IntType, false));
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, lyric_common::TypeDef::noReturn()));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdTimeTrap::TIMEZONE_CTOR));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return {};
}
