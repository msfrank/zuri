/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_FS_FILE_FILE_REF_H
#define ZURI_FS_FILE_FILE_REF_H

#include <uv.h>

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>

class FileRef : public lyric_runtime::BaseRef {

public:
    explicit FileRef(const lyric_runtime::VirtualTable *vtable);
    ~FileRef() override;

    lyric_runtime::DataCell getField(const lyric_runtime::DataCell &field) const override;
    lyric_runtime::DataCell setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value) override;
    std::string toString() const override;

    enum class State {
        Initial,
        Open,
        Closed,
        Error,
    };

    State getState() const;

    std::filesystem::path getPath() const;
    void setPath(const std::filesystem::path &path);

    tempo_utils::Status open(int flags, int mode, lyric_runtime::SystemScheduler *systemScheduler);
    tempo_utils::Status readAsync(int size, AbstractRef *fut, lyric_runtime::SystemScheduler *systemScheduler);
    tempo_utils::Status writeAsync(
        lyric_runtime::BytesRef *bytes,
        tu_int64 offset,
        AbstractRef *fut,
        lyric_runtime::SystemScheduler *systemScheduler);
    tempo_utils::Status close(lyric_runtime::SystemScheduler *systemScheduler);

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    std::filesystem::path m_path;
    State m_state;
    uv_file m_file;
    tempo_utils::Status m_status;
};

tempo_utils::Status fs_file_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status fs_file_ctor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status fs_file_create(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status fs_file_open(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status fs_file_read(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status fs_file_write(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status fs_file_close(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

#endif // ZURI_FS_FILE_FILE_REF_H
