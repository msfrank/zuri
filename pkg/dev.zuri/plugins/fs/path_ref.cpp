/* SPDX-License-Identifier: BSD-3-Clause */

#include <absl/strings/substitute.h>

#include <lyric_runtime/operand.h>
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

tu_uint64
PathRef::getTypeTag() const
{
    return type_tag();
}

bool
PathRef::utf8Value(std::string &utf8) const
{
    utf8 = m_path.string();
    return true;
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

bool
PathRef::isAbsolute() const
{
    return m_path.is_absolute();
}

bool
PathRef::isRelative() const
{
    return m_path.is_relative();
}

std::filesystem::path
PathRef::getFileName() const
{
    return m_path.filename();
}

std::filesystem::path
PathRef::getFileStem() const
{
    return m_path.stem();
}

std::string
PathRef::getFileExtension() const
{
    return m_path.extension().string();
}

std::filesystem::path
PathRef::getParentPath() const
{
    return m_path.parent_path();
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
    PathRef *instance;
    TU_ASSERT(receiver.castRef(instance));

    std::filesystem::path path;

    if (frame.numArguments() == 0) {
        if (frame.numRest() == 0) {
            path = std::filesystem::current_path();
        } else {
            for (int i = 0; i < frame.numRest(); i++) {
                auto part = frame.getRest(i);
                lyric_runtime::StringRef *str;
                TU_ASSERT (part.getString(str));
                std::string p = str->getString();
                if (!p.empty()) {
                    path /= std::filesystem::path(p);
                }
            }
        }
    } else {
        auto arg0 = frame.getArgument(0);
        lyric_runtime::StringRef *str;
        TU_ASSERT (arg0.getString(str));
        std::string p = str->getString();
        path = std::filesystem::path(p);
    }

    instance->setPath(path);

    return {};
}

tempo_utils::Status
fs_path_parent(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    PathRef *instance;
    TU_ASSERT(receiver.castRef(instance));

    auto path = instance->getParentPath();
    auto parent = state->heapManager()->allocateRef<PathRef>(instance->getVirtualTable());
    PathRef *ref;
    parent.castRef(ref);
    ref->setPath(path);

    TU_RETURN_IF_NOT_OK (currentCoro->pushData(parent));

    return {};
}

tempo_utils::Status
fs_path_resolve(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    PathRef *instance;
    TU_ASSERT(receiver.castRef(instance));

    auto path = instance->getPath();

    for (int i = 0; i < frame.numRest(); i++) {
        auto part = frame.getRest(i);
        std::string utf8;
        switch (part.getType()) {
            case lyric_runtime::OperandType::String: {
                lyric_runtime::StringRef *str;
                TU_ASSERT (part.getString(str));
                str->utf8Value(utf8);
                break;
            }
            case lyric_runtime::OperandType::Ref: {
                lyric_runtime::BaseRef *ref;
                TU_ASSERT (part.getRef(ref));
                ref->utf8Value(utf8);
                break;
            }
            default:
                break;
        }
        if (!utf8.empty()) {
            path /= utf8;
        }
    }

    auto base = instance->getPath();
    path = path.lexically_normal();

    auto resolve = state->heapManager()->allocateRef<PathRef>(instance->getVirtualTable());
    PathRef *ref;
    TU_ASSERT (resolve.castRef(ref));
    ref->setPath(path);

    TU_RETURN_IF_NOT_OK (currentCoro->pushData(resolve));

    return {};
}

tempo_utils::Status
fs_path_file_name(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    PathRef *instance;
    TU_ASSERT (receiver.castRef(instance));

    auto path = instance->getFileName();
    auto filename = state->heapManager()->allocateRef<PathRef>(instance->getVirtualTable());
    PathRef *ref;
    TU_ASSERT (filename.castRef(ref));
    ref->setPath(path);

    TU_RETURN_IF_NOT_OK (currentCoro->pushData(filename));

    return {};
}

tempo_utils::Status
fs_path_file_stem(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    PathRef *instance;
    TU_ASSERT (receiver.castRef(instance));

    auto path = instance->getFileStem();
    auto stem = state->heapManager()->allocateRef<PathRef>(instance->getVirtualTable());
    PathRef *ref;
    TU_ASSERT (stem.castRef(ref));
    ref->setPath(path);

    TU_RETURN_IF_NOT_OK (currentCoro->pushData(stem));

    return {};
}

tempo_utils::Status
fs_path_file_extension(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();
    auto *heapManager = state->heapManager();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    PathRef *instance;
    TU_ASSERT (receiver.castRef(instance));

    auto extension = instance->getFileExtension();
    TU_RETURN_IF_NOT_OK (heapManager->loadStringOntoStack(extension));

    return {};
}

tempo_utils::Status
fs_path_is_absolute(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    PathRef *instance;
    TU_ASSERT (receiver.castRef(instance));

    auto result = lyric_runtime::Operand::fromBool(instance->isAbsolute());
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(result));

    return {};
}

tempo_utils::Status
fs_path_is_relative(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    PathRef *instance;
    TU_ASSERT (receiver.castRef(instance));

    auto result = lyric_runtime::Operand::fromBool(instance->isRelative());
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(result));

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
    PathRef *instance;
    TU_ASSERT (receiver.castRef(instance));

    auto path = instance->getPath();
    TU_RETURN_IF_NOT_OK (heapManager->loadStringOntoStack(path.c_str()));

    return {};
}
