/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_SYSTEM_FUTURE_REF_H
#define ZURI_STD_SYSTEM_FUTURE_REF_H

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/gc_heap.h>
#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/promise.h>

class FutureRef;

enum class FutureState {
    Initial,
    Ready,
    Waiting,
    Resolved,
};

//enum class ResolveStatus {
//    Invalid,
//    Completed,
//    Rejected,
//    Cancelled,
//    Failed,
//};
//
//typedef tempo_utils::Result<ResolveStatus> (*ResolveCallback)(
//    lyric_runtime::DataCell &,
//    lyric_runtime::BytecodeInterpreter *,
//    lyric_runtime::InterpreterState *,
//    FutureRef *,
//    void *);

class FutureRef : public lyric_runtime::BaseRef {

public:
    explicit FutureRef(const lyric_runtime::VirtualTable *vtable);
    ~FutureRef() override;

    lyric_runtime::DataCell getField(const lyric_runtime::DataCell &field) const override;
    lyric_runtime::DataCell setField(
        const lyric_runtime::DataCell &field,
        const lyric_runtime::DataCell &value) override;
    bool prepareFuture(std::shared_ptr<lyric_runtime::Promise> promise) override;
    bool awaitFuture(lyric_runtime::SystemScheduler *systemScheduler) override;
    bool resolveFuture(lyric_runtime::DataCell &result) override;
    std::string toString() const override;

    bool isFinished() const;
    std::shared_ptr<lyric_runtime::Promise> getPromise() const;

    tempo_utils::Status forward(uv_async_t *async);
    tempo_utils::Status complete(const lyric_runtime::DataCell &result);
    tempo_utils::Status reject(const lyric_runtime::DataCell &result);

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    FutureState m_state;
    std::shared_ptr<lyric_runtime::Promise> m_promise;
    std::vector<uv_async_t *> m_deps;

    void checkState();
};

tempo_utils::Status future_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state);

tempo_utils::Status future_ctor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state);

tempo_utils::Status future_complete(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state);

tempo_utils::Status future_reject(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state);

tempo_utils::Status future_cancel(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state);

tempo_utils::Status future_then(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state);

#endif // ZURI_STD_SYSTEM_FUTURE_REF_H