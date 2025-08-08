/* SPDX-License-Identifier: AGPL-3.0-or-later */

#ifndef ZURI_EPHEMERAL_SESSION_H
#define ZURI_EPHEMERAL_SESSION_H

#include <string>

#include <lyric_build/lyric_builder.h>
#include <lyric_parser/lyric_parser.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/interpreter_state.h>

#include "abstract_session.h"
#include "fragment_store.h"

class EphemeralSession : public AbstractSession {

public:
    EphemeralSession(
        const std::string &sessionId,
        std::unique_ptr<lyric_parser::LyricParser> &&parser,
        std::unique_ptr<lyric_build::LyricBuilder> &&builder,
        std::shared_ptr<FragmentStore> fragmentStore,
        std::shared_ptr<lyric_runtime::InterpreterState> interpreterState);

    std::string_view sessionId() const override;

    tempo_utils::Result<tempo_utils::Url> parseLine(std::string_view line) override;
    tempo_utils::Result<lyric_common::ModuleLocation> compileFragment(
        const tempo_utils::Url &fragmentUrl) override;
    tempo_utils::Result<lyric_runtime::DataCell> executeFragment(
        const lyric_common::ModuleLocation &location) override;

private:
    std::string m_sessionId;
    lyric_build::TaskSettings m_taskSettings;
    std::unique_ptr<lyric_parser::LyricParser> m_parser;
    std::unique_ptr<lyric_build::LyricBuilder> m_builder;
    std::shared_ptr<FragmentStore> m_fragmentStore;
    std::shared_ptr<lyric_runtime::InterpreterState> m_interpreterState;
    std::string m_fragment;
};

#endif // ZURI_EPHEMERAL_SESSION_H