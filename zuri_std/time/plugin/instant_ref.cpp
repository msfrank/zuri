/* SPDX-License-Identifier: BSD-3-Clause */

#include <absl/strings/substitute.h>

#include <tempo_utils/log_stream.h>
#include <tempo_utils/unicode.h>

#include "instant_ref.h"

InstantRef::InstantRef(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable)
{
}

InstantRef::~InstantRef()
{
    TU_LOG_INFO << "free InstantRef" << InstantRef::toString();
}

lyric_runtime::DataCell
InstantRef::getField(const lyric_runtime::DataCell &field) const
{
    return {};
}

lyric_runtime::DataCell
InstantRef::setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value)
{
    return {};
}

std::string
InstantRef::toString() const
{
    return absl::Substitute("<$0: InstantRef $1>", this, absl::FormatTime(m_instant));
}

absl::Time
InstantRef::getInstant() const
{
    return m_instant;
}

void
InstantRef::setInstant(absl::Time instant)
{
    m_instant = instant;
}

lyric_runtime::DataCell
InstantRef::toEpochMillis() const
{
    return lyric_runtime::DataCell(static_cast<tu_int64>(absl::ToUnixMillis(m_instant)));
}

void
InstantRef::setMembersReachable()
{
}

void
InstantRef::clearMembersReachable()
{
}

tempo_utils::Status
instant_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();
    const auto *vtable = frame.getVirtualTable();
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<InstantRef>(vtable);
    currentCoro->pushData(ref);

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
instant_ctor(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver != nullptr);
    auto *instance = static_cast<InstantRef *>(receiver);
    instance->setInstant(absl::UnixEpoch());

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
instant_to_epoch_millis(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT (frame.numArguments() == 0);
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver != nullptr);
    auto *instance = static_cast<InstantRef *>(receiver);
    currentCoro->pushData(instance->toEpochMillis());
    return lyric_runtime::InterpreterStatus::ok();
}