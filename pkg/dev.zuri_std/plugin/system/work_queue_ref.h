/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_SYSTEM_WORK_QUEUE_REF_H
#define ZURI_STD_SYSTEM_WORK_QUEUE_REF_H

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/gc_heap.h>
#include <lyric_runtime/interpreter_state.h>

#include "future_ref.h"

class WorkQueueRef : public lyric_runtime::BaseRef {

public:
    WorkQueueRef( const lyric_runtime::VirtualTable *vtable);
    ~WorkQueueRef() override;

    lyric_runtime::DataCell getField(const lyric_runtime::DataCell &field) const override;
    lyric_runtime::DataCell setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value) override;
    std::string toString() const override;

    bool push(const lyric_runtime::DataCell &element);

    bool containsAvailableElement() const;
    lyric_runtime::DataCell takeAvailableElement();

    bool waitForPush(std::shared_ptr<lyric_runtime::Promise> promise, uv_async_t *async);
    bool completePop();

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    std::vector<lyric_runtime::DataCell> m_elements;
    std::vector<std::pair<std::shared_ptr<lyric_runtime::Promise>, uv_async_t *>> m_waiting;
};

tempo_utils::Status work_queue_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state);

tempo_utils::Status work_queue_push(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state);

tempo_utils::Status work_queue_pop(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state);

#endif // ZURI_STD_SYSTEM_WORK_QUEUE_REF_H