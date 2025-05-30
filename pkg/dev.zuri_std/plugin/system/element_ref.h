/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_SYSTEM_ELEMENT_REF_H
#define ZURI_STD_SYSTEM_ELEMENT_REF_H

#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/gc_heap.h>

#include "base_value_ref.h"

class ElementRef : public BaseValueRef {

public:
    ElementRef(
        const lyric_runtime::VirtualTable *vtable,
        lyric_runtime::BytecodeInterpreter *interp,
        lyric_runtime::InterpreterState *state);
    ~ElementRef() override;

    lyric_runtime::DataCell getField(const lyric_runtime::DataCell &field) const override;
    lyric_runtime::DataCell setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value) override;
    bool serializeValue(lyric_serde::PatchsetState &state, tu_uint32 &index) override;
    std::string toString() const override;

    ValueType getValueType() const override;

    void initialize(std::vector<lyric_runtime::DataCell> &&children);
    lyric_runtime::DataCell getChild(int index) const;
    int numChildren() const;

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    lyric_runtime::BytecodeInterpreter *m_interp;
    lyric_runtime::InterpreterState *m_state;
    std::vector<lyric_runtime::DataCell> m_fields;
    std::vector<lyric_runtime::DataCell> m_children;
};

tempo_utils::Status element_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status element_ctor(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status element_get_or_else(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status element_size(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);

#endif // ZURI_STD_SYSTEM_ELEMENT_REF_H