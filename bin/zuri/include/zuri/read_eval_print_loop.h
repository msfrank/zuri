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
    explicit ReadEvalPrintLoop(std::shared_ptr<AbstractSession> session);
    virtual ~ReadEvalPrintLoop();

    virtual ReadEvalPrintState getState() const;

    virtual tempo_utils::Status configure();

    virtual InputMode getMode() const;
    virtual void setMode(InputMode mode);

    virtual bool isIncomplete() const;
    virtual void setIncomplete();
    virtual void clearIncomplete();

    virtual tempo_utils::Status run();
    virtual void stop();

private:
    std::shared_ptr<AbstractSession> m_session;

    // state
    ReadEvalPrintState m_state;
    EditLine *m_editline;
    History *m_history;
    InputMode m_mode;
    bool m_isIncomplete;
};

#endif // ZURI_READ_EVAL_PRINT_LOOP_H
