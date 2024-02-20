/* SPDX-License-Identifier: AGPL-3.0-or-later */

#ifndef ZURI_READ_EVAL_PRINT_LOOP_H
#define ZURI_READ_EVAL_PRINT_LOOP_H

#include <histedit.h>

#include "ephemeral_session.h"

enum class ReadEvalPrintState {
    Initial,
    Ready,
    Running,
    Finished,
};

enum class InputMode {
    Invalid,
    Insert,
    Command,
};

class ReadEvalPrintLoop {

public:
    explicit ReadEvalPrintLoop(EphemeralSession *session);
    ~ReadEvalPrintLoop();

    ReadEvalPrintState getState() const;

    tempo_utils::Status configure();

    InputMode getMode() const;
    void setMode(InputMode mode);

    bool isIncomplete() const;
    void setIncomplete();
    void clearIncomplete();

    tempo_utils::Status run();
    void stop();

private:
    EphemeralSession *m_session;

    // state
    ReadEvalPrintState m_state;
    EditLine *m_editline;
    History *m_history;
    InputMode m_mode;
    bool m_isIncomplete;
};

#endif // ZURI_READ_EVAL_PRINT_LOOP_H
