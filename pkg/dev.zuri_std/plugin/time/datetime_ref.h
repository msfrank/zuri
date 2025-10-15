/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_TIME_DATETIME_REF_H
#define ZURI_STD_TIME_DATETIME_REF_H

#include <absl/time/time.h>

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>

class DatetimeRef : public lyric_runtime::BaseRef {

public:
    explicit DatetimeRef(const lyric_runtime::VirtualTable *vtable);
    ~DatetimeRef() override;

    std::string toString() const override;

    absl::CivilSecond getCivilSecond() const;
    void setCivilSecond(absl::CivilSecond cs);
    absl::Duration getSubseconds() const;
    void setSubseconds(absl::Duration ss);

private:
    absl::CivilSecond m_cs;
    absl::Duration m_ss;
};

tempo_utils::Status std_time_datetime_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status std_time_datetime_ctor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

#endif // ZURI_STD_TIME_DATETIME_REF_H
