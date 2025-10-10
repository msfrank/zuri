/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_TIME_TIMEZONE_REF_H
#define ZURI_STD_TIME_TIMEZONE_REF_H

#include <absl/time/time.h>

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>

class TimezoneRef : public lyric_runtime::BaseRef {

public:
    explicit TimezoneRef(const lyric_runtime::VirtualTable *vtable);
    ~TimezoneRef() override;

    lyric_runtime::DataCell getField(const lyric_runtime::DataCell &field) const override;
    lyric_runtime::DataCell setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value) override;
    std::string toString() const override;

    absl::TimeZone getTimeZone() const;
    void setTimeZone(absl::TimeZone tz);

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    absl::TimeZone m_tz;
};

tempo_utils::Status std_time_timezone_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status std_time_timezone_ctor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

#endif // ZURI_STD_TIME_TIMEZONE_REF_H
