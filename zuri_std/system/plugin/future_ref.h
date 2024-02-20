/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_SYSTEM_FUTURE_REF_H
#define ZURI_STD_SYSTEM_FUTURE_REF_H

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/gc_heap.h>
#include <lyric_runtime/interpreter_state.h>

class FutureRef;

enum class FutureState {
    Initial,
    Ready,
    Waiting,
    Completed,
};

enum class ResolveStatus {
    Invalid,
    Completed,
    Rejected,
    Cancelled,
    Failed,
};

typedef tempo_utils::Result<ResolveStatus> (*ResolveCallback)(
    lyric_runtime::DataCell &,
    lyric_runtime::BytecodeInterpreter *,
    lyric_runtime::InterpreterState *,
    FutureRef *,
    void *);

class FutureRef : public lyric_runtime::BaseRef {

public:
    explicit FutureRef(const lyric_runtime::VirtualTable *vtable);
    ~FutureRef() override;

    lyric_runtime::DataCell getField(const lyric_runtime::DataCell &field) const override;
    lyric_runtime::DataCell setField(
        const lyric_runtime::DataCell &field,
        const lyric_runtime::DataCell &value) override;
    bool attachWaiter(lyric_runtime::Waiter *waiter) override;
    bool releaseWaiter(lyric_runtime::Waiter **waiter) override;
    bool resolveFuture(
        lyric_runtime::DataCell &result,
        lyric_runtime::BytecodeInterpreter *interp,
        lyric_runtime::InterpreterState *state) override;
    std::string toString() const override;

    FutureState getState() const;
    ResolveStatus getResolveStatus() const;
    lyric_runtime::DataCell getResult() const;
    bool complete(const lyric_runtime::DataCell &result);
    bool complete(ResolveCallback cb, void *data);

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    FutureState m_state;
    lyric_runtime::Waiter *m_waiter;
    ResolveCallback m_cb;
    void *m_data;
    lyric_runtime::DataCell m_result;
    ResolveStatus m_resolveStatus;
};

tempo_utils::Status future_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state);

tempo_utils::Status future_ctor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state);

tempo_utils::Status future_resolve(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state);

tempo_utils::Status future_reject(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state);

tempo_utils::Status future_cancel(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state);

#endif // ZURI_STD_SYSTEM_FUTURE_REF_H