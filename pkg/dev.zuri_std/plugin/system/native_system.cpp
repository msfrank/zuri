/* SPDX-License-Identifier: BSD-3-Clause */

#include <tempo_utils/log_stream.h>

#include "attr_ref.h"
#include "element_ref.h"
#include "future_ref.h"
#include "native_system.h"
#include "operation_ref.h"
#include "port_ref.h"
#include "queue_ref.h"
#include "system_traps.h"

std::array<lyric_runtime::NativeTrap,26> kStdSystemTraps = {{
    {attr_alloc, "STD_SYSTEM_ATTR_ALLOC", 0},
    {element_alloc, "STD_SYSTEM_ELEMENT_ALLOC", 0},
    {element_ctor, "STD_SYSTEM_ELEMENT_CTOR", 0},
    {element_get_or_else, "STD_SYSTEM_ELEMENT_GET_OR_ELSE", 0},
    {element_size, "STD_SYSTEM_ELEMENT_SIZE", 0},
    {future_alloc, "STD_SYSTEM_FUTURE_ALLOC", 0},
    {future_ctor, "STD_SYSTEM_FUTURE_CTOR", 0},
    {future_complete, "STD_SYSTEM_FUTURE_COMPLETE", 0},
    {future_reject, "STD_SYSTEM_FUTURE_REJECT", 0},
    {future_cancel, "STD_SYSTEM_FUTURE_CANCEL", 0},
    {future_then, "STD_SYSTEM_FUTURE_THEN", 0},
    {append_operation_alloc, "STD_SYSTEM_APPEND_OPERATION_ALLOC", 0},
    {insert_operation_alloc, "STD_SYSTEM_INSERT_OPERATION_ALLOC", 0},
    {update_operation_alloc, "STD_SYSTEM_UPDATE_OPERATION_ALLOC", 0},
    {replace_operation_alloc, "STD_SYSTEM_REPLACE_OPERATION_ALLOC", 0},
    {emit_operation_alloc, "STD_SYSTEM_EMIT_OPERATION_ALLOC", 0},
    {port_receive, "STD_SYSTEM_PORT_RECEIVE", 0},
    {port_send, "STD_SYSTEM_PORT_SEND", 0},
    {queue_alloc, "STD_SYSTEM_QUEUE_ALLOC", 0},
    {queue_pop, "STD_SYSTEM_QUEUE_POP", 0},
    {queue_push, "STD_SYSTEM_QUEUE_PUSH", 0},
    {std_system_acquire, "STD_SYSTEM_ACQUIRE", 0},
    {std_system_await, "STD_SYSTEM_AWAIT", 0},
    {std_system_get_result, "STD_SYSTEM_GET_RESULT", 0},
    {std_system_sleep, "STD_SYSTEM_SLEEP", 0},
    {std_system_spawn, "STD_SYSTEM_SPAWN", 0},
}};

class NativeStdSystem : public lyric_runtime::NativeInterface {

public:
    NativeStdSystem() = default;
    bool load(lyric_runtime::BytecodeSegment *segment) const override;
    void unload(lyric_runtime::BytecodeSegment *segment) const override;
    const lyric_runtime::NativeTrap *getTrap(uint32_t index) const override;
    uint32_t numTraps() const override;
};

const lyric_runtime::NativeTrap *
NativeStdSystem::getTrap(uint32_t index) const
{
    if (kStdSystemTraps.size() <= index)
        return nullptr;
    return &kStdSystemTraps.at(index);
}

bool
NativeStdSystem::load(lyric_runtime::BytecodeSegment *segment) const
{
    return true;
}

void
NativeStdSystem::unload(lyric_runtime::BytecodeSegment *segment) const
{
}

uint32_t
NativeStdSystem::numTraps() const
{
    return kStdSystemTraps.size();
}

static const NativeStdSystem iface;

const lyric_runtime::NativeInterface *native_init()
{
    return &iface;
}
