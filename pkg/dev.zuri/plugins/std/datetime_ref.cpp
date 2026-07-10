/* SPDX-License-Identifier: BSD-3-Clause */

#include <absl/strings/substitute.h>

#include <tempo_utils/log_stream.h>
#include <tempo_utils/unicode.h>

#include "datetime_ref.h"
#include "instant_ref.h"
#include "timezone_ref.h"

DatetimeRef::DatetimeRef(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable)
{
}

DatetimeRef::~DatetimeRef()
{
    TU_LOG_INFO << "free DatetimeRef" << DatetimeRef::toString();
}

tu_uint64
DatetimeRef::getTypeTag() const
{
    return type_tag();
}

std::string
DatetimeRef::toString() const
{
    return absl::Substitute("<$0: Datetime $1>", this, absl::FormatCivilTime(m_cs));
}

absl::CivilSecond
DatetimeRef::getCivilSecond() const
{
    return m_cs;
}

void
DatetimeRef::setCivilSecond(absl::CivilSecond cs)
{
    m_cs = cs;
}

absl::Duration
DatetimeRef::getSubseconds() const
{
    return m_ss;
}

void
DatetimeRef::setSubseconds(absl::Duration ss)
{
    m_ss = ss;
}

tempo_utils::Status
std_time_datetime_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    TU_ASSERT(vtable != nullptr);
    auto *currentCoro = state->currentCoro();
    auto ref = state->heapManager()->allocateRef<DatetimeRef>(vtable);
    currentCoro->pushData(ref);

    return {};
}

tempo_utils::Status
std_time_datetime_ctor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    TU_ASSERT (frame.numArguments() == 2);
    auto arg0 = frame.getArgument(0);
    InstantRef *instant;
    TU_ASSERT (arg0.castRef(instant));
    auto arg1 = frame.getArgument(1);
    TimezoneRef *tz;
    TU_ASSERT (arg1.castRef(tz));
    auto receiver = frame.getReceiver();
    DatetimeRef *datetime;
    TU_ASSERT(receiver.castRef(datetime));

    auto i = instant->getInstant();
    auto z = tz->getTimeZone();
    auto cs = absl::ToCivilSecond(i, z);
    auto ci = z.At(i);
    auto ss = ci.subsecond;

    datetime->setCivilSecond(cs);
    datetime->setSubseconds(ss);

    return {};
}