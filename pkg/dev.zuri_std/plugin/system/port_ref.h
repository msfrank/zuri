/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_SYSTEM_PORT_REF_H
#define ZURI_STD_SYSTEM_PORT_REF_H

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/gc_heap.h>
#include <lyric_runtime/interpreter_state.h>

class PortRef : public lyric_runtime::BaseRef {

public:
    explicit PortRef(const lyric_runtime::VirtualTable *vtable);
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

    std::shared_ptr<lyric_runtime::DuplexPort> duplexPort();

    bool send(std::shared_ptr<tempo_utils::ImmutableBytes> payload);
    bool waitForReceive(std::shared_ptr<lyric_runtime::Promise> promise, uv_async_t *async);

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    lyric_runtime::BytecodeInterpreter *m_interp;
    lyric_runtime::InterpreterState *m_state;
    std::shared_ptr<lyric_runtime::DuplexPort> m_port;
    std::vector<lyric_runtime::DataCell> m_values;
};

tempo_utils::Status port_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status port_send(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status port_receive(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

#endif // ZURI_STD_SYSTEM_PORT_REF_H