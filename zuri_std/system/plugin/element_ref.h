/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_SYSTEM_ELEMENT_REF_H
#define ZURI_STD_SYSTEM_ELEMENT_REF_H

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/gc_heap.h>
#include <lyric_runtime/interpreter_state.h>

class ElementRef : public lyric_runtime::BaseRef {

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

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    lyric_runtime::BytecodeInterpreter *m_interp;
    lyric_runtime::InterpreterState *m_state;
    std::vector<lyric_runtime::DataCell> m_fields;
};

tempo_utils::Status element_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state);

#endif // ZURI_STD_SYSTEM_ELEMENT_REF_H