/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_CONN_CONNECTION_REF_H
#define ZURI_STD_CONN_CONNECTION_REF_H

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>

class ConnectionRef : public lyric_runtime::BaseRef {

public:
    explicit ConnectionRef(const lyric_runtime::VirtualTable *vtable);
    ~ConnectionRef() override;

    std::string toString() const override;

    std::shared_ptr<lyric_runtime::Connection> getConnection() const;
    void setConnection(std::shared_ptr<lyric_runtime::Connection> connection);

    tempo_utils::Status send(std::shared_ptr<const tempo_utils::ImmutableBytes> payload);
    tempo_utils::Status shutdown();

private:
    std::shared_ptr<lyric_runtime::Connection> m_conn;
};

tempo_utils::Status std_connection_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status std_connection_send(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status std_connection_receive(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

#endif // ZURI_STD_CONN_CONNECTION_REF_H
