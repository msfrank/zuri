/* SPDX-License-Identifier: AGPL-3.0-or-later */

#include <iostream>
#include <memory>

#include <histedit.h>

#include <lyric_parser/lyric_parser.h>
#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <tempo_command/command_help.h>

#include <zuri_run/ephemeral_session.h>
#include <zuri_run/read_eval_print_loop.h>
#include <zuri_run/run_result.h>

static const char *editline_prog        = "zuri-shell";
static const char *command_prompt       = "zuri: ";
static const char *insert_prompt        = "zuri> ";
static const char *incomplete_prompt    = "  ... ";
static const char *result_prompt        = "  --> ";

zuri_run::ReadEvalPrintLoop::ReadEvalPrintLoop(std::shared_ptr<AbstractSession> session)
    : m_session(std::move(session)),
      m_editline(nullptr),
      m_history(nullptr),
      m_mode(InputMode::Insert),
      m_isIncomplete(false)
{
    TU_ASSERT (m_session != nullptr);
}

zuri_run::ReadEvalPrintLoop::~ReadEvalPrintLoop()
{
    history_end(m_history);
    el_end(m_editline);
}

static const char *
prompt_func(EditLine *el)
{
    void *data = nullptr;
    if (el_get(el, EL_CLIENTDATA, &data) < 0)
        throw std::runtime_error("failed to retrieve client data");
    auto *repl = (zuri_run::ReadEvalPrintLoop *) data;

    if (repl->isIncomplete())
        return incomplete_prompt;
    switch (repl->getMode()) {
        case zuri_run::InputMode::Insert:
            return insert_prompt;
        case zuri_run::InputMode::Command:
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
    auto *repl = (zuri_run::ReadEvalPrintLoop *) data;

    switch (repl->getMode()) {
        case zuri_run::InputMode::Insert:
            repl->setMode(zuri_run::InputMode::Command);
            break;
        case zuri_run::InputMode::Command:
            repl->setMode(zuri_run::InputMode::Insert);
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
    auto *repl = (zuri_run::ReadEvalPrintLoop *) data;

    repl->stop();

    return CC_EOF;
}

tempo_utils::Status
zuri_run::ReadEvalPrintLoop::configure()
{
    if (m_editline != nullptr)
        return RunStatus::forCondition(RunCondition::kRunInvariant,
            "repl is already configured");

    // initialize editline
    auto *edit = el_init(editline_prog, stdin, stdout, stderr);
    el_set(edit, EL_CLIENTDATA, this);
    el_set(edit, EL_PROMPT, &prompt_func);
    el_set(edit, EL_EDITOR, "vi");

    // initialize history
    auto *hist = history_init();
    HistEvent hist_event;
    history(hist, &hist_event, H_SETSIZE, 1000);
    el_set(edit, EL_HIST, history, hist);

    // add builtins
    el_set(edit, EL_ADDFN, "lyric-switch-mode", "switch between command and insert mode", builtin_switch_mode);
    el_set(edit, EL_ADDFN, "lyric-exit-shell", "exit the lyric shell", builtin_exit_shell);

    // set bindings
    el_set(edit, EL_BIND, "^A", "lyric-switch-mode", nullptr);

    m_editline = edit;
    m_history = hist;

    return {};
}

zuri_run::InputMode
zuri_run::ReadEvalPrintLoop::getMode() const
{
    return m_mode;
}

void
zuri_run::ReadEvalPrintLoop::setMode(InputMode mode)
{
    m_mode = mode;
}

bool
zuri_run::ReadEvalPrintLoop::isIncomplete() const
{
    return m_isIncomplete;
}

void
zuri_run::ReadEvalPrintLoop::setIncomplete()
{
    m_isIncomplete = true;
}

void
zuri_run::ReadEvalPrintLoop::clearIncomplete()
{
    m_isIncomplete = false;
}

tempo_utils::Status
zuri_run::ReadEvalPrintLoop::run()
{
    // loop forever until ctrl-D (EOF) or unrecoverable error
    for (;;) {

        // read the current line
        int count = 0;
        const char *buffer = el_gets(m_editline, &count);

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
        std::string line(buffer, count);
        auto parseLineResult = m_session->parseLine(line);
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
        auto fragmentUrl = parseLineResult.getResult();

        // store the complete code fragment in the history
        HistEvent histEvent;
        history(m_history, &histEvent, H_ENTER, line.c_str());
        clearIncomplete();

        // compile the code fragment
        auto compileFragmentResult = m_session->compileFragment(fragmentUrl);
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
zuri_run::ReadEvalPrintLoop::stop()
{
}