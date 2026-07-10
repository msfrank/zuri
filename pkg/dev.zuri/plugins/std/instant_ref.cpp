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

tu_uint64
InstantRef::getTypeTag() const
{
    return type_tag();
}

std::string
InstantRef::toString() const
{
    return absl::Substitute("<$0: Instant $1>", this, absl::FormatTime(m_instant));
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

lyric_runtime::Operand
InstantRef::toEpochMillis() const
{
    auto epochMillis = static_cast<tu_int64>(absl::ToUnixMillis(m_instant));
    return lyric_runtime::Operand::fromI64(epochMillis);
}

tempo_utils::Status
std_time_instant_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    TU_ASSERT(vtable != nullptr);
    auto *currentCoro = state->currentCoro();
    auto ref = state->heapManager()->allocateRef<InstantRef>(vtable);
    currentCoro->pushData(ref);
    return {};
}

tempo_utils::Status
std_time_instant_ctor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    InstantRef *instance;
    TU_ASSERT(receiver.castRef(instance));
    instance->setInstant(absl::UnixEpoch());

    return {};
}

tempo_utils::Status
std_time_instant_to_epoch_millis(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT (frame.numArguments() == 0);
    auto receiver = frame.getReceiver();
    InstantRef *instance;
    TU_ASSERT(receiver.castRef(instance));
    currentCoro->pushData(instance->toEpochMillis());
    return {};
}