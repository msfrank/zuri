/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_DATA_DATA_REF_H
#define ZURI_STD_DATA_DATA_REF_HH

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <tempo_utils/gap_buffer.h>

class DataRef : public lyric_runtime::BaseRef {

public:
    explicit DataRef(const lyric_runtime::VirtualTable *vtable);
    ~DataRef() override;

    std::string toString() const override;

private:
    tempo_utils::GapBuffer<tu_uint8> m_buf;
};

tempo_utils::Status std_data_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status std_data_ctor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

#endif // ZURI_STD_DATA_DATA_REF_HH
