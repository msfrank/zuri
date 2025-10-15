/* SPDX-License-Identifier: BSD-3-Clause */

#include <absl/strings/substitute.h>

#include <lyric_runtime/data_cell.h>
#include <lyric_runtime/heap_manager.h>
#include <lyric_runtime/string_ref.h>
#include <tempo_utils/log_stream.h>
#include <tempo_utils/unicode.h>

#include "path_ref.h"

PathRef::PathRef(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable)
{
}

PathRef::~PathRef()
{
    TU_LOG_V << "free PathRef" << PathRef::toString();
}

std::string
PathRef::toString() const
{
    return absl::Substitute("<$0: Path $1>", this, m_path.string());
}

std::filesystem::path
PathRef::getPath() const
{
    return m_path;
}

void
PathRef::setPath(const std::filesystem::path &path)
{
    m_path = path;
}

tempo_utils::Status
fs_path_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    TU_ASSERT(vtable != nullptr);
    auto *currentCoro = state->currentCoro();
    auto ref = state->heapManager()->allocateRef<PathRef>(vtable);
    currentCoro->pushData(ref);
    return {};
}

tempo_utils::Status
fs_path_ctor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<PathRef *>(receiver.data.ref);

    std::filesystem::path path;

    if (frame.numArguments() == 0) {
        if (frame.numRest() == 0) {
            path = std::filesystem::current_path();
        } else {
            for (int i = 0; i < frame.numRest(); i++) {
                auto part = frame.getRest(i);
                TU_ASSERT (part.type == lyric_runtime::DataCellType::STRING);
                auto *str = part.data.str;
                std::string_view sv(str->getStringData(), str->getStringSize());
                path /= std::filesystem::path(sv);
            }
        }
    } else {
        auto arg0 = frame.getArgument(0);
        TU_ASSERT (arg0.type == lyric_runtime::DataCellType::STRING);
        auto *str = arg0.data.str;
        std::string_view sv(str->getStringData(), str->getStringSize());
        path = std::filesystem::path(sv);
    }

    path = std::filesystem::absolute(path);
    instance->setPath(path);

    return {};
}

tempo_utils::Status
fs_path_to_string(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();
    auto *heapManager = state->heapManager();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<PathRef *>(receiver.data.ref);

    auto path = instance->getPath();
    TU_RETURN_IF_NOT_OK (heapManager->loadStringOntoStack(path.c_str()));

    return {};
}
