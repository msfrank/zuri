/* SPDX-License-Identifier: AGPL-3.0-or-later */

#ifndef ZURI_RUN_ABSTRACT_SESSION_H
#define ZURI_RUN_ABSTRACT_SESSION_H

#include <string>

#include <lyric_common/module_location.h>

namespace zuri_run {

    class AbstractSession {

    public:
        virtual ~AbstractSession() = default;

        virtual std::string_view sessionId() const = 0;

        virtual tempo_utils::Result<tempo_utils::Url> parseLine(std::string_view line) = 0;

        virtual tempo_utils::Result<lyric_common::ModuleLocation> compileFragment(
            const tempo_utils::Url &fragmentUrl) = 0;

        virtual tempo_utils::Result<lyric_runtime::DataCell> executeFragment(
            const lyric_common::ModuleLocation &location) = 0;
    };
}

#endif // ZURI_RUN_ABSTRACT_SESSION_H