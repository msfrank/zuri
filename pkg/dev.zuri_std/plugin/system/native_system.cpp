/* SPDX-License-Identifier: BSD-3-Clause */

#include <tempo_utils/log_stream.h>
#include <zuri_std_system/lib_types.h>

#include "attr_ref.h"
#include "element_ref.h"
#include "future_ref.h"
#include "native_system.h"
#include "operation_ref.h"
#include "port_ref.h"
#include "queue_ref.h"
#include "system_traps.h"

lyric_runtime::NativeFunc
NativeStdSystem::getTrap(uint32_t index) const
{
    if (index >= static_cast<uint32_t>(StdSystemTrap::LAST_))
        return nullptr;
    auto trapFunction = static_cast<StdSystemTrap>(index);
    switch (trapFunction) {
        case StdSystemTrap::ATTR_ALLOC:
            return attr_alloc;
        case StdSystemTrap::ELEMENT_ALLOC:
            return element_alloc;
        case StdSystemTrap::ELEMENT_CTOR:
            return element_ctor;
        case StdSystemTrap::ELEMENT_GET_OR_ELSE:
            return element_get_or_else;
        case StdSystemTrap::ELEMENT_SIZE:
            return element_size;
        case StdSystemTrap::FUTURE_ALLOC:
            return future_alloc;
        case StdSystemTrap::FUTURE_CTOR:
            return future_ctor;
        case StdSystemTrap::FUTURE_COMPLETE:
            return future_complete;
        case StdSystemTrap::FUTURE_REJECT:
            return future_reject;
        case StdSystemTrap::FUTURE_CANCEL:
            return future_cancel;
        case StdSystemTrap::FUTURE_THEN:
            return future_then;
        case StdSystemTrap::APPEND_OPERATION_ALLOC:
            return append_operation_alloc;
        case StdSystemTrap::INSERT_OPERATION_ALLOC:
            return insert_operation_alloc;
        case StdSystemTrap::UPDATE_OPERATION_ALLOC:
            return update_operation_alloc;
        case StdSystemTrap::REPLACE_OPERATION_ALLOC:
            return replace_operation_alloc;
        case StdSystemTrap::EMIT_OPERATION_ALLOC:
            return emit_operation_alloc;
        case StdSystemTrap::PORT_RECEIVE:
            return port_receive;
        case StdSystemTrap::PORT_SEND:
            return port_send;
        case StdSystemTrap::QUEUE_ALLOC:
            return queue_alloc;
        case StdSystemTrap::QUEUE_POP:
            return queue_pop;
        case StdSystemTrap::QUEUE_PUSH:
            return queue_push;
        case StdSystemTrap::ACQUIRE:
            return std_system_acquire;
        case StdSystemTrap::AWAIT:
            return std_system_await;
        case StdSystemTrap::GET_RESULT:
            return std_system_get_result;
        case StdSystemTrap::SLEEP:
            return std_system_sleep;
        case StdSystemTrap::SPAWN:
            return std_system_spawn;
        case StdSystemTrap::LAST_:
            break;
    }
    TU_UNREACHABLE();
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
    return static_cast<uint32_t>(StdSystemTrap::LAST_);
}

static const NativeStdSystem iface;

const lyric_runtime::NativeInterface *native_init()
{
    return &iface;
}
