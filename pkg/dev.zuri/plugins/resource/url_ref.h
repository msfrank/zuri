#ifndef ZURI_STD_RESOURCE_URL_REF_H
#define ZURI_STD_RESOURCE_URL_REF_H

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/interpreter_state.h>

class UrlRef : public lyric_runtime::BaseRef {

public:
    explicit UrlRef(const lyric_runtime::VirtualTable *vtable);
    ~UrlRef() override;

    tempo_utils::Url getUrl() const;
    void setUrl(const tempo_utils::Url &url);

    std::string toString() const override;

private:
    tempo_utils::Url m_url;
};

tempo_utils::Status std_url_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status std_url_equality_equals(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status std_url_to_string(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status std_parse_url(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

#endif // ZURI_STD_RESOURCE_URL_REF_H
