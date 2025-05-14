/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_TIME_DATETIME_REF_H
#define ZURI_STD_TIME_DATETIME_REF_H

#include <absl/time/time.h>

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>

class DatetimeRef : public lyric_runtime::BaseRef {

public:
    DatetimeRef(const lyric_runtime::VirtualTable *vtable);
    ~DatetimeRef() override;

    lyric_runtime::DataCell getField(const lyric_runtime::DataCell &field) const override;
    lyric_runtime::DataCell setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value) override;
    std::string toString() const override;

    absl::CivilSecond getCivilSecond() const;
    void setCivilSecond(absl::CivilSecond cs);
    absl::Duration getSubseconds() const;
    void setSubseconds(absl::Duration ss);

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    absl::CivilSecond m_cs;
    absl::Duration m_ss;
};

tempo_utils::Status datetime_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state);

tempo_utils::Status datetime_ctor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state);

#endif // ZURI_STD_TIME_DATETIME_REF_H
