/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_TIME_INSTANT_REF_H
#define ZURI_STD_TIME_INSTANT_REF_H

#include <absl/time/time.h>

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>

class InstantRef : public lyric_runtime::BaseRef {

public:
    explicit InstantRef(const lyric_runtime::VirtualTable *vtable);
    ~InstantRef() override;

    std::string toString() const override;

    absl::Time getInstant() const;
    void setInstant(absl::Time instant);

    lyric_runtime::DataCell toEpochMillis() const;

private:
    absl::Time m_instant;
};

tempo_utils::Status std_time_instant_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status std_time_instant_ctor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status std_time_instant_to_epoch_millis(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

#endif // ZURI_STD_TIME_INSTANT_REF_H
