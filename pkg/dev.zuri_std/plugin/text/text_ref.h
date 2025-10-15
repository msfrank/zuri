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

    std::string toString() const override;

    lyric_runtime::DataCell textAt(int index) const;
    lyric_runtime::DataCell textSize() const;

    const UChar *getTextData() const;
    int32_t getTextSize() const;
    bool setTextData(const char *data, int32_t size);

private:
    UChar *m_data;
    tu_int32 m_size;
};

tempo_utils::Status std_text_text_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status std_text_text_ctor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status std_text_text_length(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status std_text_text_at(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

#endif // ZURI_STD_TEXT_TEXT_REF_H
