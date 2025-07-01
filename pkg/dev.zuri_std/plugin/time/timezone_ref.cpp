/* SPDX-License-Identifier: BSD-3-Clause */

#include <absl/strings/substitute.h>

#include <tempo_utils/log_stream.h>
#include <tempo_utils/unicode.h>

#include "timezone_ref.h"

TimezoneRef::TimezoneRef(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable)
{
}

TimezoneRef::~TimezoneRef()
{
    TU_LOG_INFO << "free TimezoneRef" << TimezoneRef::toString();
}

lyric_runtime::DataCell
TimezoneRef::getField(const lyric_runtime::DataCell &field) const
{
    return {};
}

lyric_runtime::DataCell
TimezoneRef::setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value)
{
    return {};
}

std::string
TimezoneRef::toString() const
{
    return absl::Substitute("<$0: Timezone $1>", this, m_tz.name());
}

absl::TimeZone
TimezoneRef::getTimeZone() const
{
    return m_tz;
}

void
TimezoneRef::setTimeZone(absl::TimeZone tz)
{
    m_tz = tz;
}

void
TimezoneRef::setMembersReachable()
{
}

void
TimezoneRef::clearMembersReachable()
{
}

tempo_utils::Status
std_time_timezone_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    const auto *vtable = frame.getVirtualTable();
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<TimezoneRef>(vtable);
    currentCoro->pushData(ref);

    return {};
}

tempo_utils::Status
std_time_timezone_ctor(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    TU_ASSERT (frame.numArguments() == 1);
    auto arg0 = frame.getArgument(0);
    TU_ASSERT (arg0.type == lyric_runtime::DataCellType::I64);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<TimezoneRef *>(receiver.data.ref);

    int offsetSeconds = 0;
    if (0 <= arg0.data.i64 && arg0.data.i64 < 60 * 60 * 24) {
        offsetSeconds = static_cast<int>(arg0.data.i64);
    }
    instance->setTimeZone(absl::FixedTimeZone(offsetSeconds));

    return {};
}