/* SPDX-License-Identifier: BSD-3-Clause */

#include <iostream>

#include <lyric_runtime/data_cell.h>
#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/string_ref.h>

#include "timezone_ref.h"
#include "time_traps.h"
#include "instant_ref.h"

tempo_utils::Status
std_time_now(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *unused)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    TU_ASSERT(frame.numArguments() == 0);

    auto *segmentManager = state->segmentManager();
    auto *heapManager = state->heapManager();

    auto *segment = currentCoro->peekSP();
    auto object = segment->getObject();
    auto symbol = object.findSymbol(lyric_common::SymbolPath({"Instant"}));
    TU_ASSERT (symbol.isValid());

    lyric_runtime::InterpreterStatus status;
    auto descriptor = segmentManager->resolveDescriptor(segment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    TU_ASSERT (descriptor.type == lyric_runtime::DataCellType::CLASS);
    const auto *vtable = segmentManager->resolveClassVirtualTable(descriptor, status);
    TU_ASSERT(vtable != nullptr);

    // create a new instant instance
    auto ref = heapManager->allocateRef<InstantRef>(vtable);
    currentCoro->pushData(ref);

    // set the timezone on the instance
    auto *instance = static_cast<InstantRef *>(ref.data.ref);
    instance->setInstant(absl::Now());

    return {};
}

tempo_utils::Status
std_time_parse_timezone(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *unused)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() >= 1);
    const auto &cell = frame.getArgument(0);
    TU_ASSERT(cell.type == lyric_runtime::DataCellType::STRING);

    // get the timezone name argument
    std::string tzName;
    if (!cell.data.str->utf8Value(tzName))
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid timezone name");

    // load the timezone data
    absl::TimeZone tz;
    if (!absl::LoadTimeZone(tzName, &tz))
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "failed to load timezone {}", tzName);

    auto *segmentManager = state->segmentManager();
    auto *heapManager = state->heapManager();

    auto *segment = currentCoro->peekSP();
    auto object = segment->getObject();
    auto symbol = object.findSymbol(lyric_common::SymbolPath({"Timezone"}));
    TU_ASSERT (symbol.isValid());

    lyric_runtime::InterpreterStatus status;
    auto descriptor = segmentManager->resolveDescriptor(segment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    TU_ASSERT (descriptor.type == lyric_runtime::DataCellType::CLASS);
    const auto *vtable = segmentManager->resolveClassVirtualTable(descriptor, status);
    TU_ASSERT(vtable != nullptr);

    // create a new timezone instance
    auto ref = heapManager->allocateRef<TimezoneRef>(vtable);
    currentCoro->pushData(ref);

    // set the timezone on the instance
    auto *instance = static_cast<TimezoneRef *>(ref.data.ref);
    instance->setTimeZone(tz);

    return {};
}
