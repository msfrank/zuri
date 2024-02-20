/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_TEXT_TEXT_REF_H
#define ZURI_STD_TEXT_TEXT_REF_H

#include <unicode/ustring.h>

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>

class TextRef : public lyric_runtime::BaseRef {

public:
    explicit TextRef(const lyric_runtime::VirtualTable *vtable);
    TextRef(const lyric_runtime::VirtualTable *vtable, const UChar *data, int32_t size);
    ~TextRef() override;

    lyric_runtime::DataCell getField(const lyric_runtime::DataCell &field) const override;
    lyric_runtime::DataCell setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value) override;
    std::string toString() const override;

    lyric_runtime::DataCell textAt(int index) const;
    lyric_runtime::DataCell textSize() const;

    const UChar *getTextData() const;
    int32_t getTextSize() const;
    bool setTextData(const char *data, int32_t size);

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    UChar *m_data;
    tu_int32 m_size;
};

tempo_utils::Status text_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state);

tempo_utils::Status text_ctor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state);

tempo_utils::Status text_size(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state);

tempo_utils::Status text_at(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state);

#endif // ZURI_STD_TEXT_TEXT_REF_H
