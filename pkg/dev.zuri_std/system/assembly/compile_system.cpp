/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_typing/callsite_reifier.h>
#include <zuri_std_system/lib_types.h>

#include "compile_system.h"

tempo_utils::Status
build_std_system(
    lyric_assembler::AssemblyState &state,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem)
{
    auto *fundamentalCache = state.fundamentalCache();
    auto *symbolCache = state.symbolCache();
    auto *typeCache = state.typeCache();

    auto UrlSpec = lyric_parser::Assignable::forSingular({"Url"});
    auto PortSpec = lyric_parser::Assignable::forSingular({"Port"});
    auto TSpec = lyric_parser::Assignable::forSingular({"T"});
    auto FutureTSpec = lyric_parser::Assignable::forSingular(
        lyric_common::SymbolPath({"Future"}), {TSpec});
    auto StatusOrTSpec = lyric_parser::Assignable::forUnion({
        lyric_parser::Assignable::forSingular(
            fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Status)),
        TSpec});
    auto Function0ReturningTSpec = lyric_parser::Assignable::forSingular(
        fundamentalCache->getFunctionUrl(0), {TSpec});

    {
        auto declareFunctionResult = block->declareFunction("Acquire",
            {
                { {}, "uri", "", UrlSpec, lyric_parser::BindingType::VALUE },
            },
            {},
            {},
            PortSpec,
            lyric_object::AccessType::Public,
            {});
        auto functionUrl = declareFunctionResult.getResult();
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(functionUrl));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<uint32_t>(StdSystemTrap::ACQUIRE));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareFunctionResult = block->declareFunction("Await",
            {
                { {}, "fut", "", FutureTSpec, lyric_parser::BindingType::VALUE }
            },
            {},
            {},
            StatusOrTSpec,
            lyric_object::AccessType::Public,
            {
                {
                    "T",
                    0,
                    fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any),
                    lyric_object::VarianceType::Invariant,
                    lyric_object::BoundType::None,
                }
            });
        auto functionUrl = declareFunctionResult.getResult();
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(functionUrl));
        auto *code = call->callProc()->procCode();

        // after the await trap completes the current coro may have been suspended
        code->trap(static_cast<uint32_t>(StdSystemTrap::AWAIT));

        // if the coro was suspended it is resumed at this instruction. after the future_result trap
        // completes, the result will be on the top of the stack.
        code->trap(static_cast<uint32_t>(StdSystemTrap::GET_RESULT));

        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareFunctionResult = block->declareFunction("AwaitOrDefault",
            {
                { {}, "fut", "", FutureTSpec, lyric_parser::BindingType::VALUE },
                { {}, "default", "", TSpec, lyric_parser::BindingType::VALUE },
            },
            {},
            {},
            TSpec,
            lyric_object::AccessType::Public,
            {
                {
                    "T",
                    0,
                    fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any),
                    lyric_object::VarianceType::Invariant,
                    lyric_object::BoundType::None,
                }
            });
        auto functionUrl = declareFunctionResult.getResult();
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(functionUrl));
        auto *proc = call->callProc();
        auto *code = proc->procCode();

        // after the await trap completes the current coro may have been suspended
        TU_RETURN_IF_NOT_OK (code->trap(static_cast<uint32_t>(StdSystemTrap::AWAIT)));

        // if the coro was suspended it is resumed at this instruction. after the future_result trap
        // completes, the result will be on the top of the stack.
        TU_RETURN_IF_NOT_OK (code->trap(static_cast<uint32_t>(StdSystemTrap::GET_RESULT)));

        // allocate a local and store the result in it
        auto result = proc->allocateLocal();
        TU_RETURN_IF_NOT_OK (code->storeLocal(result));

        // load the result onto and push its type onto the top of the stack
        TU_RETURN_IF_NOT_OK (code->loadLocal(result));
        TU_RETURN_IF_NOT_OK (code->writeOpcode(lyric_object::Opcode::OP_TYPE_OF));

        // get the type handle for Status and push the Status type onto the top of the stack
        auto statusType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Status);
        auto *statusTypeHandle = typeCache->getType(statusType);
        TU_ASSERT (statusTypeHandle != nullptr);
        statusTypeHandle->touch();
        TU_RETURN_IF_NOT_OK (code->loadType(statusTypeHandle->getAddress()));

        // perform type comparison result isA Status
        TU_RETURN_IF_NOT_OK (code->writeOpcode(lyric_object::Opcode::OP_TYPE_CMP));

        // if lhs type equals or extends rhs, then jump to defaultLabel
        lyric_assembler::PatchOffset predicateJump;
        TU_ASSIGN_OR_RETURN (predicateJump, code->jumpIfLessOrEqual());

        // if we did not jump then result was not status, so return result
        TU_RETURN_IF_NOT_OK (code->loadLocal(result));
        TU_RETURN_IF_NOT_OK (code->writeOpcode(lyric_object::Opcode::OP_RETURN));

        // patch predicate jump to default label
        lyric_assembler::JumpLabel defaultLabel;
        TU_ASSIGN_OR_RETURN (defaultLabel, code->makeLabel());
        TU_RETURN_IF_NOT_OK (code->patch(predicateJump, defaultLabel));

        // result is a status, so return the default argument
        TU_RETURN_IF_NOT_OK (code->loadArgument(lyric_assembler::ArgumentOffset(1)));
        TU_RETURN_IF_NOT_OK (code->writeOpcode(lyric_object::Opcode::OP_RETURN));
    }
    {
        auto declareFunctionResult = block->declareFunction("Sleep",
            {
                {   {},
                    "millis",
                    "",
                    lyric_parser::Assignable::forSingular({"Int"}),
                    lyric_parser::BindingType::VALUE
                }
            },
            {},
            {},
            lyric_parser::Assignable::forSingular({"Future"},
                {lyric_parser::Assignable::forSingular({"Nil"})}),  // FIXME: type system shouldn't allow this
            lyric_object::AccessType::Public,
            {});
        auto functionUrl = declareFunctionResult.getResult();
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(functionUrl));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<uint32_t>(StdSystemTrap::SLEEP));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }
    {
        auto declareFunctionResult = block->declareFunction("Spawn",
            {
                {   {},
                    "fn",
                    "",
                    Function0ReturningTSpec,
                    lyric_parser::BindingType::VALUE
                }
            },
            {},
            {},
            FutureTSpec,
            lyric_object::AccessType::Public,
            {
                {
                    "T",
                    0,
                    fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any),
                    lyric_object::VarianceType::Invariant,
                    lyric_object::BoundType::None,
                }
            });
        auto functionUrl = declareFunctionResult.getResult();
        auto *call = cast_symbol_to_call(symbolCache->getSymbol(functionUrl));
        auto *code = call->callProc()->procCode();
        code->trap(static_cast<uint32_t>(StdSystemTrap::SPAWN));
        code->writeOpcode(lyric_object::Opcode::OP_RETURN);
    }

    return tempo_utils::Status();
}