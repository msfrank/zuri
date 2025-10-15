#ifndef ZURI_FS_FILE_PATH_REF_H
#define ZURI_FS_FILE_PATH_REF_H

#include <uv.h>

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>

class PathRef : public lyric_runtime::BaseRef {

public:
    explicit PathRef(const lyric_runtime::VirtualTable *vtable);
    ~PathRef() override;

    std::string toString() const override;

    std::filesystem::path getPath() const;
    void setPath(const std::filesystem::path &path);

private:
    std::filesystem::path m_path;
};

tempo_utils::Status fs_path_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status fs_path_ctor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status fs_path_to_string(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

#endif // ZURI_FS_FILE_PATH_REF_H