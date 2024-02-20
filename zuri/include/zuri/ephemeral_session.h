/* SPDX-License-Identifier: AGPL-3.0-or-later */

#ifndef ZURI_EPHEMERAL_SESSION_H
#define ZURI_EPHEMERAL_SESSION_H

#include <filesystem>
#include <string>

#include <lyric_build/lyric_builder.h>
#include <lyric_parser/lyric_parser.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/interpreter_state.h>

#include "fragment_store.h"

class EphemeralSession {

public:
    EphemeralSession(
        const std::string &sessionId,
        const lyric_build::ConfigStore &configStore,
        std::shared_ptr<FragmentStore> fragmentStore,
        const lyric_parser::ParserOptions &parserOptions = {});

    std::string getSessionId() const;

    tempo_utils::Status configure();

    tempo_utils::Result<std::string> parseLine(const char *data, size_t size);
    tempo_utils::Result<lyric_common::AssemblyLocation> compileFragment(const std::string &fragment);
    tempo_utils::Result<lyric_runtime::Return> executeFragment(const lyric_common::AssemblyLocation &location);

private:
    std::string m_sessionId;
    lyric_build::ConfigStore m_configStore;
    std::shared_ptr<FragmentStore> m_fragmentStore;
    lyric_parser::ParserOptions m_parserOptions;
    std::unique_ptr<lyric_parser::LyricParser> m_parser;
    std::unique_ptr<lyric_build::LyricBuilder> m_builder;
    std::shared_ptr<lyric_runtime::InterpreterState> m_state;
    std::string m_fragment;
};

#endif // ZURI_EPHEMERAL_SESSION_H