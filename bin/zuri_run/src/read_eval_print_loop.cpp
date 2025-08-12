/* SPDX-License-Identifier: AGPL-3.0-or-later */

#include <iostream>
#include <memory>

#include <histedit.h>
#include <absl/strings/ascii.h>

#include <lyric_parser/lyric_parser.h>
#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <tempo_command/command_help.h>

#include <zuri_run/ephemeral_session.h>
#include <zuri_run/read_eval_print_loop.h>
#include <zuri_run/run_result.h>

static const char *editline_prog        = "zuri-run";
static const char *normal_prompt        = "zuri> ";
static const char *incomplete_prompt    = "  ... ";
static const char *result_prompt        = "  --> ";

zuri_run::ReadEvalPrintLoop::ReadEvalPrintLoop(std::shared_ptr<AbstractSession> session)
    : m_session(std::move(session)),
      m_editline(nullptr),
      m_history(nullptr),
      m_complete(true)
{
    TU_ASSERT (m_session != nullptr);
}

zuri_run::ReadEvalPrintLoop::~ReadEvalPrintLoop()
{
    TU_LOG_FATAL_IF (m_editline || m_history) << "repl was not cleaned up properly";
}

static const char *
prompt_func(EditLine *el)
{
    void *data = nullptr;
    if (el_get(el, EL_CLIENTDATA, &data) < 0)
        throw std::runtime_error("failed to retrieve client data");

    auto *repl = (zuri_run::ReadEvalPrintLoop *) data;
    if (repl->isComplete())
        return normal_prompt;

    return incomplete_prompt;
}

// static unsigned char
// builtin_exit_shell(EditLine *el, int ch)
// {
//     TU_CONSOLE_ERR << "builtin_exit_shell";
//
//     void *data = nullptr;
//     if (el_get(el, EL_CLIENTDATA, &data) < 0)
//         throw std::runtime_error("failed to retrieve client data");
//     return CC_EOF;
// }

bool
zuri_run::ReadEvalPrintLoop::isComplete() const
{
    return m_complete;
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
    el_set(edit, EL_SIGNAL, 1);

    // initialize history
    auto *hist = history_init();
    HistEvent hist_event;
    history(hist, &hist_event, H_SETSIZE, 1000);
    el_set(edit, EL_HIST, history, hist);

    // add zuri-exit-shell function and bind it to ctrl-D
    //el_set(edit, EL_ADDFN, "zuri-exit-shell", "exit the lyric shell", builtin_exit_shell);
    //el_set(edit, EL_BIND, "^D", "zuri-exit-shell", nullptr);

    // set bindings

    m_editline = edit;
    m_history = hist;

    return {};
}

tempo_utils::Status
zuri_run::ReadEvalPrintLoop::run()
{
    int count = 0;
    const char *buffer = nullptr;

    // loop forever until ctrl-D (EOF) or unrecoverable error
    while ((buffer = el_gets(m_editline, &count)) != nullptr) {

        // if buffer length is 0 then do nothing
        if (count == 0)
            continue;

        // if line contains only whitespace then do nothing
        std::string line(buffer, count);
        absl::StripAsciiWhitespace(&line);
        if (line.empty())
            continue;

        // otherwise we are in insert mode, parse line as a code fragment
        auto parseLineResult = m_session->parseLine(line);
        if (parseLineResult.isStatus()) {
            auto status = parseLineResult.getStatus();
            if (status.matchesCondition(lyric_parser::ParseCondition::kIncompleteModule)) {
                m_complete = false;
            } else if (status.getErrorCategory() == lyric_parser::kLyricParserStatusNs) {
                TU_CONSOLE_ERR << "parse error: " << status;
            } else {
                return status;
            }
        }
        auto fragmentUrl = parseLineResult.getResult();

        // store the complete code fragment in the history
        HistEvent histEvent;
        history(m_history, &histEvent, H_ENTER, line.c_str());
        m_complete = true;

        // compile the code fragment
        auto compileFragmentResult = m_session->compileFragment(fragmentUrl);
        if (compileFragmentResult.isStatus()) {
            TU_CONSOLE_ERR << "compile error: " << compileFragmentResult.getStatus();
            continue;
        }

        // execute the code fragment
        auto location = compileFragmentResult.getResult();
        auto executeFragmentResult = m_session->executeFragment(location);
        if (executeFragmentResult.isStatus()) {
            TU_CONSOLE_ERR << "interpreter error: " << executeFragmentResult.getStatus();
            continue;
        }

        // print result
        auto ret = executeFragmentResult.getResult();
        std::cout << result_prompt << ret.toString() << std::endl;
    }

    // cleanup and exit
    return {};
}

tempo_utils::Status
zuri_run::ReadEvalPrintLoop::cleanup()
{
    if (m_editline != nullptr) {
        el_end(m_editline);
        history_end(m_history);
        m_editline = nullptr;
        m_history = nullptr;
    }

    TU_CONSOLE_OUT << "";

    return {};
}