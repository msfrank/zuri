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

lyric_runtime::DataCell
DatetimeRef::getField(const lyric_runtime::DataCell &field) const
{
    return {};
}

lyric_runtime::DataCell
DatetimeRef::setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value)
{
    return {};
}

std::string
DatetimeRef::toString() const
{
    return absl::Substitute("<$0: DatetimeRef $1>", this, absl::FormatCivilTime(m_cs));
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

void
DatetimeRef::setMembersReachable()
{
}

void
DatetimeRef::clearMembersReachable()
{
}

tempo_utils::Status
datetime_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();
    const auto *vtable = frame.getVirtualTable();
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<DatetimeRef>(vtable);
    currentCoro->pushData(ref);

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
datetime_ctor(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();
    TU_ASSERT (frame.numArguments() == 2);
    auto arg0 = frame.getArgument(0);
    TU_ASSERT (arg0.type == lyric_runtime::DataCellType::REF);
    auto arg1 = frame.getArgument(1);
    TU_ASSERT (arg1.type == lyric_runtime::DataCellType::REF);
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver != nullptr);

    auto instant = static_cast<InstantRef *>(arg0.data.ref)->getInstant();
    auto tz = static_cast<TimezoneRef *>(arg1.data.ref)->getTimeZone();
    auto cs = absl::ToCivilSecond(instant, tz);
    auto ci = tz.At(instant);
    auto ss = ci.subsecond;

    auto *instance = static_cast<DatetimeRef *>(receiver);
    instance->setCivilSecond(cs);
    instance->setSubseconds(ss);

    return lyric_runtime::InterpreterStatus::ok();
}