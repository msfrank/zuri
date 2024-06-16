/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/pack_builder.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_typing/callsite_reifier.h>
#include <zuri_std_time/lib_types.h>

#include "compile_time.h"

tempo_utils::Status
build_std_time(
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem)
{
    auto *fundamentalCache = state.fundamentalCache();

    auto StringType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::String);
    auto InstantType = lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl::fromString("#Instant"));
    auto TimezoneType = lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl::fromString("#Timezone"));

    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, block->declareFunction("Now", lyric_object::AccessType::Public, {}));
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall({}, InstantType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<uint32_t>(StdTimeTrap::NOW));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, block->declareFunction("ParseTimezone", lyric_object::AccessType::Public, {}));
        lyric_assembler::PackBuilder packBuilder;
        packBuilder.appendListParameter("name", {}, StringType, false);
        lyric_assembler::ParameterPack parameterPack;
        TU_ASSIGN_OR_RETURN (parameterPack, packBuilder.toParameterPack());
        lyric_assembler::ProcHandle *procHandle;
        TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, TimezoneType));
        auto *codeBuilder = procHandle->procCode();
        codeBuilder->trap(static_cast<uint32_t>(StdTimeTrap::PARSE_TIMEZONE));
        codeBuilder->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return {};
}