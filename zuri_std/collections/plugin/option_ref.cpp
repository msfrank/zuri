/* SPDX-License-Identifier: BSD-3-Clause */

#include <absl/strings/substitute.h>

#include "option_ref.h"

OptionRef::OptionRef(const lyric_runtime::VirtualTable *vtable)
    : lyric_runtime::BaseRef(vtable)
{
    TU_ASSERT (vtable != nullptr);
}

OptionRef::~OptionRef()
{
    TU_LOG_INFO << "free " << OptionRef::toString();
}

lyric_runtime::DataCell
OptionRef::getField(const lyric_runtime::DataCell &field) const
{
    return {};
}

lyric_runtime::DataCell
OptionRef::setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value)
{
    return {};
}

bool
OptionRef::hashValue(absl::HashState state)
{
    switch (m_value.type) {
        case lyric_runtime::DataCellType::INVALID:
            absl::HashState::combine(std::move(state), 0);
            return true;
        case lyric_runtime::DataCellType::NIL:
            absl::HashState::combine(std::move(state), 1);
            return true;
        case lyric_runtime::DataCellType::BOOL:
            absl::HashState::combine(std::move(state), m_value.data.b);
            return true;
        case lyric_runtime::DataCellType::CHAR32:
            absl::HashState::combine(std::move(state), m_value.data.chr);
            return true;
        case lyric_runtime::DataCellType::I64:
            absl::HashState::combine(std::move(state), m_value.data.i64);
            return true;
        case lyric_runtime::DataCellType::DBL:
            absl::HashState::combine(std::move(state), m_value.data.dbl);
            return true;
        case lyric_runtime::DataCellType::UTF8:
            absl::HashState::combine_contiguous(std::move(state), m_value.data.utf8.data, m_value.data.utf8.size);
            return true;
        case lyric_runtime::DataCellType::TYPE:
        case lyric_runtime::DataCellType::CONCEPT:
        case lyric_runtime::DataCellType::FIELD:
        case lyric_runtime::DataCellType::CALL:
        case lyric_runtime::DataCellType::CLASS:
        case lyric_runtime::DataCellType::STRUCT:
        case lyric_runtime::DataCellType::INSTANCE:
        case lyric_runtime::DataCellType::ENUM:
        case lyric_runtime::DataCellType::ACTION:
        case lyric_runtime::DataCellType::EXISTENTIAL:
        case lyric_runtime::DataCellType::NAMESPACE:
            absl::HashState::combine(std::move(state), m_value.data.descriptor.assembly, m_value.data.descriptor.value);
            return true;
        case lyric_runtime::DataCellType::REF:
            m_value.data.ref->hashValue(absl::HashState::Create(&state));
            return true;
        default:
            return false;
    }
}

std::string
OptionRef::toString() const
{
    return absl::Substitute("<$0: OptionRef>", this);
}

lyric_runtime::DataCell
OptionRef::optionGet() const
{
    return m_value;
}

bool
OptionRef::optionSet(const lyric_runtime::DataCell &value)
{
    if (m_value.isValid())
        return false;
    m_value = value;
    return true;
}

void
OptionRef::setMembersReachable()
{
    if (m_value.type == lyric_runtime::DataCellType::REF)
        m_value.data.ref->setReachable();
}

void
OptionRef::clearMembersReachable()
{
    if (m_value.type == lyric_runtime::DataCellType::REF)
        m_value.data.ref->clearReachable();
}

tempo_utils::Status
option_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();
    const auto *vtable = frame.getVirtualTable();
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<OptionRef>(vtable);
    currentCoro->pushData(ref);

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
option_ctor(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<OptionRef *>(receiver.data.ref);

    TU_ASSERT (frame.numArguments() == 1);
    const auto &arg0 = frame.getArgument(0);

    if (!instance->optionSet(arg0))
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "option has invalid state");

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
option_get(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT (frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TU_ASSERT (receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<OptionRef *>(receiver.data.ref);
    auto value = instance->optionGet();

    TU_ASSERT (value.isValid());
    currentCoro->pushData(value);
    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
option_is_empty(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<OptionRef *>(receiver.data.ref);
    auto value = instance->optionGet();

    TU_ASSERT (value.isValid());
    lyric_runtime::DataCell result(value.type == lyric_runtime::DataCellType::NIL);
    currentCoro->pushData(result);

    return lyric_runtime::InterpreterStatus::ok();
}
