/* SPDX-License-Identifier: AGPL-3.0-or-later */

#include <iostream>
#include <memory>

#include <histedit.h>

#include <lyric_parser/lyric_parser.h>
#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <tempo_command/command_help.h>

#include <zuri/ephemeral_session.h>
#include <zuri/read_eval_print_loop.h>

static const char *editline_prog        = "zuri-shell";
static const char *command_prompt       = "zuri: ";
static const char *insert_prompt        = "zuri> ";
static const char *incomplete_prompt    = "  ... ";
static const char *result_prompt        = "  --> ";

//const char *current_prompt = insert_prompt;
//bool is_command_mode = false;
//bool is_incomplete = false;
//bool running = false;

ReadEvalPrintLoop::ReadEvalPrintLoop(EphemeralSession *session)
    : m_session(session),
      m_state(ReadEvalPrintState::Initial),
      m_editline(nullptr),
      m_history(nullptr),
      m_mode(InputMode::Insert),
      m_isIncomplete(false)
{
    TU_ASSERT (m_session != nullptr);
}

ReadEvalPrintLoop::~ReadEvalPrintLoop()
{
    history_end(m_history);
    el_end(m_editline);
}

ReadEvalPrintState
ReadEvalPrintLoop::getState() const
{
    return m_state;
}

static const char *
prompt_func(EditLine *el)
{
    void *data = nullptr;
    if (el_get(el, EL_CLIENTDATA, &data) < 0)
        throw std::runtime_error("failed to retrieve client data");
    auto *repl = (ReadEvalPrintLoop *) data;

    if (repl->isIncomplete())
        return incomplete_prompt;
    switch (repl->getMode()) {
        case InputMode::Insert:
            return insert_prompt;
        case InputMode::Command:
            return command_prompt;
        default:
            return "???";
    }
}

static unsigned char
builtin_switch_mode(EditLine *el, int ch)
{
    void *data = nullptr;
    if (el_get(el, EL_CLIENTDATA, &data) < 0)
        throw std::runtime_error("failed to retrieve client data");
    auto *repl = (ReadEvalPrintLoop *) data;

    switch (repl->getMode()) {
        case InputMode::Insert:
            repl->setMode(InputMode::Command);
            break;
        case InputMode::Command:
            repl->setMode(InputMode::Insert);
            break;
        default:
            break;
    }

    return CC_REDISPLAY;
}

static unsigned char
builtin_exit_shell(EditLine *el, int ch)
{
    void *data = nullptr;
    if (el_get(el, EL_CLIENTDATA, &data) < 0)
        throw std::runtime_error("failed to retrieve client data");
    auto *repl = (ReadEvalPrintLoop *) data;

    repl->stop();

    return CC_EOF;
}

tempo_utils::Status
ReadEvalPrintLoop::configure()
{
    if (m_state != ReadEvalPrintState::Initial)
        return tempo_utils::GenericStatus::forCondition(
            tempo_utils::GenericCondition::kInternalViolation, "invalid REPL state");

    // initialize editline
    m_editline = el_init(editline_prog, stdin, stdout, stderr);
    el_set(m_editline, EL_CLIENTDATA, this);
    el_set(m_editline, EL_PROMPT, &prompt_func);
    el_set(m_editline, EL_EDITOR, "vi");

    // initialize history
    m_history = history_init();
    HistEvent hist_event;
    history(m_history, &hist_event, H_SETSIZE, 1000);
    el_set(m_editline, EL_HIST, history, m_history);

    // add builtins
    el_set(m_editline, EL_ADDFN, "lyric-switch-mode", "switch between command and insert mode", builtin_switch_mode);
    el_set(m_editline, EL_ADDFN, "lyric-exit-shell", "exit the lyric shell", builtin_exit_shell);

    // set bindings
    el_set(m_editline, EL_BIND, "^A", "lyric-switch-mode", nullptr);

    m_state = ReadEvalPrintState::Ready;

    return {};
}

InputMode
ReadEvalPrintLoop::getMode() const
{
    return m_mode;
}

void
ReadEvalPrintLoop::setMode(InputMode mode)
{
    m_mode = mode;
}

bool
ReadEvalPrintLoop::isIncomplete() const
{
    return m_isIncomplete;
}

void
ReadEvalPrintLoop::setIncomplete()
{
    m_isIncomplete = true;
}

void
ReadEvalPrintLoop::clearIncomplete()
{
    m_isIncomplete = false;
}

tempo_utils::Status
ReadEvalPrintLoop::run()
{
    if (m_state != ReadEvalPrintState::Ready)
        return tempo_utils::GenericStatus::forCondition(
            tempo_utils::GenericCondition::kInternalViolation, "invalid REPL state");

    TU_LOG_V << "starting shell";

    // set repl state to running
    m_state = ReadEvalPrintState::Running;

    // loop forever until ctrl-D (EOF) or unrecoverable error
    for (;;) {

        // read the current line
        int count = 0;
        const char *buffer = el_gets(m_editline, &count);

        // if user has signaled EOF then mark repl as finished
        if (m_state == ReadEvalPrintState::Finished)
            break;

        // if buffer length is 0 or gets returns nullptr then do nothing
        if (count == 0 || buffer == nullptr)
            continue;

        // if we are in command mode, then parse and invoke builtin
        if (getMode() == InputMode::Command) {
            Tokenizer *tok = tok_init(nullptr);
            const char **cmdv;
            int cmdc;
            tok_str(tok, buffer, &cmdc, &cmdv);
            if (cmdc <= 0)
                continue;
            auto elStatus = el_parse(m_editline, cmdc, cmdv);
            if (elStatus < 0) {
                std::cerr << "unknown builtin " << cmdv[0] << std::endl;
            } else if (elStatus > 0) {
                std::cerr << "builtin failed" << std::endl;
            }
            tok_end(tok);
            continue;
        }

        // otherwise we are in insert mode, parse line as a code fragment
        auto parseLineResult = m_session->parseLine(buffer, count);
        if (parseLineResult.isStatus()) {
            lyric_parser::ParseStatus parseStatus;
            if (!parseLineResult.getStatus().convertTo(parseStatus))
                return parseLineResult.getStatus();
            if (parseStatus.getCondition() == lyric_parser::ParseCondition::kIncompleteModule) {
                setIncomplete();
            } else {
                std::cerr << "parse error: " << parseStatus.getMessage() << std::endl;
            }
            continue;
        }

        // store the complete code fragment in the history
        auto fragment = parseLineResult.getResult();
        HistEvent histEvent;
        history(m_history, &histEvent, H_ENTER, fragment.c_str());
        clearIncomplete();

        // compile the code fragment
        auto compileFragmentResult = m_session->compileFragment(fragment);
        if (compileFragmentResult.isStatus()) {
            auto compileStatus = compileFragmentResult.getStatus();
            std::cerr << "build error: " << compileStatus.getMessage() << std::endl;
            continue;
        }

        // execute the code fragment
        auto location = compileFragmentResult.getResult();
        auto executeFragmentResult = m_session->executeFragment(location);
        if (executeFragmentResult.isStatus()) {
            auto executeStatus = executeFragmentResult.getStatus();
            std::cerr << "interpreter error: " << executeStatus.getMessage() << std::endl;
            continue;
        }

        // print result
        auto ret = executeFragmentResult.getResult();
        std::cout << result_prompt << ret.toString() << std::endl;
    }

    // cleanup and exit
    return {};
}

void
ReadEvalPrintLoop::stop()
{
    if (m_state == ReadEvalPrintState::Running) {
        m_state = ReadEvalPrintState::Finished;
    }
}