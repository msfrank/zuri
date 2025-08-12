/* SPDX-License-Identifier: AGPL-3.0-or-later */

#ifndef ZURI_RUN_READ_EVAL_PRINT_LOOP_H
#define ZURI_RUN_READ_EVAL_PRINT_LOOP_H

#include <histedit.h>

#include "ephemeral_session.h"

namespace zuri_run {

    enum class InputMode {
        Invalid,
        Insert,
        Command,
    };

    class ReadEvalPrintLoop {

    public:
        explicit ReadEvalPrintLoop(std::shared_ptr<AbstractSession> session);
        virtual ~ReadEvalPrintLoop();

        virtual tempo_utils::Status configure();

        virtual bool isComplete() const;

        virtual tempo_utils::Status run();

        virtual tempo_utils::Status cleanup();

    private:
        std::shared_ptr<AbstractSession> m_session;

        // state
        EditLine *m_editline;
        History *m_history;
        bool m_complete;
    };
}

#endif // ZURI_RUN_READ_EVAL_PRINT_LOOP_H
