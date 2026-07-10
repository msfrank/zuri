/* SPDX-License-Identifier: BSD-3-Clause */

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/operand.h>
#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/string_ref.h>
#include <tempo_utils/unicode.h>

#include "resource_traps.h"

tempo_utils::Status
std_resource_exists(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();
    auto *segmentManager = state->segmentManager();

    auto &frame = currentCoro->currentCallOrThrow();
    auto *returnSegment = segmentManager->getSegment(frame.getReturnSegment());
    auto resourceOrigin = returnSegment->getObjectLocation().getOrigin();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &arg0 = frame.getArgument(0);
    lyric_runtime::StringRef *str;
    TU_ASSERT(arg0.getString(str));

    lyric_runtime::Operand result;

    std::string path;
    if (!str->utf8Value(path))
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "utf8 conversion failed");
    auto resourcePath = tempo_utils::UrlPath::fromString(path);

    if (resourcePath.isValid()) {
        auto resourceUrl = tempo_utils::Url::fromOrigin(resourceOrigin.toString(), resourcePath.toString());
        auto resourceLocation = lyric_common::ModuleLocation::fromUrl(resourceUrl);

        tempo_utils::Status status;
        auto exists = segmentManager->hasResource(resourceLocation, /* useSystemLoader= */ false, &status);
        if (status.isOk()) {
            result = lyric_runtime::Operand::fromBool(exists);
        } else {
            result = lyric_runtime::Operand::fromBool(false);

        }
    }

    TU_RETURN_IF_NOT_OK (currentCoro->pushData(result));

    return {};
}

tempo_utils::Status
std_resource_load(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();
    auto *heapManager = state->heapManager();
    auto *segmentManager = state->segmentManager();

    auto &frame = currentCoro->currentCallOrThrow();
    auto *returnSegment = segmentManager->getSegment(frame.getReturnSegment());
    auto resourceOrigin = returnSegment->getObjectLocation().getOrigin();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &arg0 = frame.getArgument(0);
    lyric_runtime::StringRef *str;
    TU_ASSERT(arg0.getString(str));

    lyric_runtime::Operand result;
    tempo_utils::Status status;

    std::string path;
    if (!str->utf8Value(path))
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "utf8 conversion failed");
    auto resourcePath = tempo_utils::UrlPath::fromString(path);

    if (resourcePath.isValid()) {
        auto resourceUrl = tempo_utils::Url::fromOrigin(resourceOrigin.toString(), resourcePath.toString());
        auto resourceLocation = lyric_common::ModuleLocation::fromUrl(resourceUrl);

        auto bytes = segmentManager->loadResource(resourceLocation, /* useSystemLoader= */ false, &status);
        if (status.notOk()) {
            result = heapManager->allocateStatus(status.getStatusCode(), status.getMessage());
        } else {
            result = heapManager->allocateBytes(bytes->getSpan());
        }
    }

    TU_RETURN_IF_NOT_OK (currentCoro->pushData(result));

    return {};
}