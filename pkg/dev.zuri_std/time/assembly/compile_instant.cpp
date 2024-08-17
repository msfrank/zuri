/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <zuri_std_time/lib_types.h>

#include "compile_instant.h"

tempo_utils::Status
build_std_time_Instant(
    lyric_assembler::ObjectState &state,
    lyric_assembler::BlockHandle *block)
{
    auto *fundamentalCache = state.fundamentalCache();

    lyric_assembler::ClassSymbol *ObjectClass;
    TU_ASSIGN_OR_RETURN (ObjectClass, block->resolveClass(
        fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object)));

    lyric_assembler::ClassSymbol *InstantClass;
    TU_ASSIGN_OR_RETURN (InstantClass, block->declareClass(
        "Instant", ObjectClass, lyric_object::AccessType::Public, {}));

    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);

    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, InstantClass->declareCtor(
            lyric_object::AccessType::Public, static_cast<tu_uint32>(StdTimeTrap::INSTANT_ALLOC)));
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall({}, lyric_common::TypeDef::noReturn()));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdTimeTrap::INSTANT_CTOR));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, InstantClass->declareMethod(
            "ToEpochMillis", lyric_object::AccessType::Public));
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall({}, IntType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<tu_uint32>(StdTimeTrap::INSTANT_TO_EPOCH_MILLIS));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return {};
}
