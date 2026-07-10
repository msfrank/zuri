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

tu_uint64
TimezoneRef::getTypeTag() const
{
    return type_tag();
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

tempo_utils::Status
std_time_timezone_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    TU_ASSERT(vtable != nullptr);
    auto *currentCoro = state->currentCoro();
    auto ref = state->heapManager()->allocateRef<TimezoneRef>(vtable);
    currentCoro->pushData(ref);
    return {};
}

tempo_utils::Status
std_time_timezone_ctor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    TU_ASSERT (frame.numArguments() == 1);
    auto arg0 = frame.getArgument(0);
    tu_int64 offset;
    TU_ASSERT (arg0.getI64(offset));

    auto receiver = frame.getReceiver();
    TimezoneRef *instance;
    TU_ASSERT(receiver.castRef(instance));

    int offsetSeconds = 0;
    if (0 <= offset && offset < 60 * 60 * 24) {
        offsetSeconds = static_cast<int>(offset);
    }
    instance->setTimeZone(absl::FixedTimeZone(offsetSeconds));

    return {};
}