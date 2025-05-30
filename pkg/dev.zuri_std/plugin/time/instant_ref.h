/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_TIME_INSTANT_REF_H
#define ZURI_STD_TIME_INSTANT_REF_H

#include <absl/time/time.h>

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>

class InstantRef : public lyric_runtime::BaseRef {

public:
    InstantRef(const lyric_runtime::VirtualTable *vtable);
    ~InstantRef() override;

    lyric_runtime::DataCell getField(const lyric_runtime::DataCell &field) const override;
    lyric_runtime::DataCell setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value) override;
    std::string toString() const override;

    absl::Time getInstant() const;
    void setInstant(absl::Time instant);

    lyric_runtime::DataCell toEpochMillis() const;

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    absl::Time m_instant;
};

tempo_utils::Status std_time_instant_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state);

tempo_utils::Status std_time_instant_ctor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state);

tempo_utils::Status std_time_instant_to_epoch_millis(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state);

#endif // ZURI_STD_TIME_INSTANT_REF_H
