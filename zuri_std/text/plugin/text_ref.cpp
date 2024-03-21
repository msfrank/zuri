/* SPDX-License-Identifier: BSD-3-Clause */

#include <absl/strings/substitute.h>

#include <tempo_utils/log_stream.h>
#include <tempo_utils/unicode.h>

#include "text_ref.h"

TextRef::TextRef(const lyric_runtime::VirtualTable *vtable) : BaseRef(vtable)
{
    m_data = nullptr;
    m_size = 0;
}

TextRef::TextRef(const lyric_runtime::VirtualTable *vtable, const UChar *data, int32_t size) : BaseRef(vtable)
{
    TU_ASSERT (vtable != nullptr);
    TU_ASSERT (data != nullptr);
    TU_ASSERT (size >= 0);

    m_data = new UChar[size];
    m_size = size;
    u_memcpy(m_data, data, size);
}

TextRef::~TextRef()
{
    TU_LOG_INFO << "free TextRef" << TextRef::toString();
    delete[] m_data;
}

lyric_runtime::DataCell
TextRef::getField(const lyric_runtime::DataCell &field) const
{
    return {};
}

lyric_runtime::DataCell
TextRef::setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value)
{
    return {};
}

std::string
TextRef::toString() const
{
    std::string s;
    if (m_data)
        s = tempo_utils::convert_to_utf8(m_data, m_size);

    return absl::Substitute("<$0: TextRef \"$1\">", this, s);
}

lyric_runtime::DataCell
TextRef::textAt(int index) const
{
    if (m_data == nullptr)
        return lyric_runtime::DataCell::nil();
    UChar32 char32;
    U16_GET(m_data, 0, index, m_size, char32);
    return lyric_runtime::DataCell(char32);
}

lyric_runtime::DataCell
TextRef::textSize() const
{
    if (m_data == nullptr)
        return lyric_runtime::DataCell(static_cast<int64_t>(0));
    return lyric_runtime::DataCell(static_cast<int64_t>(u_countChar32(m_data, m_size)));
}

const UChar *
TextRef::getTextData() const
{
    return m_data;
}

int32_t
TextRef::getTextSize() const
{
    return m_size;
}

bool
TextRef::setTextData(const char *data, tu_int32 size)
{
    if (m_data) {
        delete m_data;
        m_size = 0;
    }

    tu_int32 destLength = 0;
    UErrorCode err = U_ZERO_ERROR;

    u_strFromUTF8(nullptr, 0, &destLength, data, size, &err);
    TU_LOG_INFO << "u_strFromUTF8 -> " << u_errorName(err);
    if (err != U_ZERO_ERROR && err != U_BUFFER_OVERFLOW_ERROR) {
        TU_LOG_INFO << "failed to perform preflight to UTF-16: " << u_errorName(err);
        return false;
    }

    m_data = new UChar[destLength];
    err = U_ZERO_ERROR;

    u_strFromUTF8(m_data, destLength, &m_size, data, size, &err);
    if (err != U_ZERO_ERROR && err != U_STRING_NOT_TERMINATED_WARNING) {
        TU_LOG_INFO << "failed to convert to UTF-16: " << u_errorName(err);
        return false;
    }

    return true;
}

void
TextRef::setMembersReachable()
{
}

void
TextRef::clearMembersReachable()
{
}

tempo_utils::Status
text_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();
    const auto *vtable = frame.getVirtualTable();
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<TextRef>(vtable);
    currentCoro->pushData(ref);

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
text_ctor(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<TextRef *>(receiver.data.ref);

    TU_ASSERT (frame.numArguments() == 1);
    const auto &arg0 = frame.getArgument(0);
    if (arg0.type == lyric_runtime::DataCellType::UTF8) {
        if (!instance->setTextData(arg0.data.utf8.data, arg0.data.utf8.size))
            return lyric_runtime::InterpreterStatus::forCondition(
                lyric_runtime::InterpreterCondition::kRuntimeInvariant,
                "utf16 conversion failed");
    } else if (arg0.type == lyric_runtime::DataCellType::REF) {
        std::string utf8;
        if (!arg0.data.ref->utf8Value(utf8))
            return lyric_runtime::InterpreterStatus::forCondition(
                lyric_runtime::InterpreterCondition::kRuntimeInvariant,
                "utf8 conversion failed");
        if (!instance->setTextData(utf8.data(), utf8.size()))
            return lyric_runtime::InterpreterStatus::forCondition(
                lyric_runtime::InterpreterCondition::kRuntimeInvariant,
                "utf16 conversion failed");
    } else {
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant,
            "invalid utf8 argument {}", arg0.toString());
    }

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
text_at(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT (frame.numArguments() == 1);
    const auto &index = frame.getArgument(0);
    TU_ASSERT(index.type == lyric_runtime::DataCellType::I64);
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<TextRef *>(receiver.data.ref);
    currentCoro->pushData(instance->textAt(index.data.i64));
    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
text_size(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<TextRef *>(receiver.data.ref);
    currentCoro->pushData(instance->textSize());
    return lyric_runtime::InterpreterStatus::ok();
}
