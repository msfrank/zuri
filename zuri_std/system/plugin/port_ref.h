/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_SYSTEM_PORT_REF_H
#define ZURI_STD_SYSTEM_PORT_REF_H

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/gc_heap.h>
#include <lyric_runtime/interpreter_state.h>

class PortRef : public lyric_runtime::BaseRef {

public:
    PortRef(
        const lyric_runtime::VirtualTable *vtable,
        lyric_runtime::BytecodeInterpreter *interp,
        lyric_runtime::InterpreterState *state);
    PortRef(
        const lyric_runtime::VirtualTable *vtable,
        lyric_runtime::BytecodeInterpreter *interp,
        lyric_runtime::InterpreterState *state,
        std::shared_ptr<lyric_runtime::DuplexPort> port);
    ~PortRef() override;

    lyric_runtime::DataCell getField(const lyric_runtime::DataCell &field) const override;
    lyric_runtime::DataCell setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value) override;
    std::string toString() const override;

    bool send(const lyric_serde::LyricPatchset &patchset);

    bool isInReceive() const;
    bool waitForReceive(FutureRef *fut, uv_async_t *async);
    bool completeReceive();

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    lyric_runtime::BytecodeInterpreter *m_interp;
    lyric_runtime::InterpreterState *m_state;
    std::shared_ptr<lyric_runtime::DuplexPort> m_port;
    std::vector<lyric_runtime::DataCell> m_values;
    FutureRef *m_fut;
};

tempo_utils::Status port_send(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state);

tempo_utils::Status port_receive(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state);

#endif // ZURI_STD_SYSTEM_PORT_REF_H