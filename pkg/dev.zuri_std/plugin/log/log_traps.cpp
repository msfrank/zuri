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

#include <tempo_utils/memory_bytes.h>

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

    auto payload = tempo_utils::MemoryBytes::copy(utf8);
    port->send(std::move(payload));

    currentCoro->pushData(lyric_runtime::DataCell(true));

    return {};
}
