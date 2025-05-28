/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/data_cell.h>
#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/string_ref.h>
#include <lyric_serde/patchset_state.h>
#include <lyric_serde/patchset_change.h>
#include <lyric_serde/patchset_value.h>
#include <tempo_utils/unicode.h>

#include "log_traps.h"

tempo_utils::Status
std_log_log(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();
    auto *multiplexer = state->portMultiplexer();

    auto protocolUrl = tempo_utils::Url::fromString("dev.zuri.proto:log");
    auto port = multiplexer->getPort(protocolUrl);
    if (port == nullptr)
        return lyric_runtime::InterpreterStatus::forCondition(lyric_runtime::InterpreterCondition::kRuntimeInvariant,
            "missing port dev.zuri.proto:log");

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &cell = frame.getArgument(0);
    TU_ASSERT(cell.type == lyric_runtime::DataCellType::STRING);
    auto *instance = cell.data.str;

    std::string utf8;
    if (!instance->utf8Value(utf8))
        return lyric_runtime::InterpreterStatus::forCondition(lyric_runtime::InterpreterCondition::kRuntimeInvariant,
            "utf8 conversion failed");

    lyric_serde::PatchsetState patchsetState;

    auto appendValueResult = patchsetState.appendValue(tempo_schema::AttrValue(utf8));
    if (appendValueResult.isStatus())
        return lyric_runtime::InterpreterStatus::forCondition(lyric_runtime::InterpreterCondition::kRuntimeInvariant,
            "failed to append value");
    auto *value = appendValueResult.getResult();

    std::string changeId;
    auto appendChangeResult = patchsetState.appendChange(changeId);
    if (appendChangeResult.isStatus())
        return lyric_runtime::InterpreterStatus::forCondition(lyric_runtime::InterpreterCondition::kRuntimeInvariant,
            "failed to append change");
    auto *change = appendChangeResult.getResult();

    change->setEmitOperation(value->getAddress());

    auto toPatchsetResult = patchsetState.toPatchset();
    if (toPatchsetResult.isStatus())
        return lyric_runtime::InterpreterStatus::forCondition(lyric_runtime::InterpreterCondition::kRuntimeInvariant,
            "failed to serialize patchset");

    port->send(toPatchsetResult.getResult());

    currentCoro->pushData(lyric_runtime::DataCell(true));

    return lyric_runtime::InterpreterStatus::ok();
}