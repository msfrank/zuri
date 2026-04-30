/* SPDX-License-Identifier: BSD-3-Clause */

#include "datetime_ref.h"
#include "future_ref.h"
#include "instant_ref.h"
#include "native_std.h"
#include "port_ref.h"
#include "process_traps.h"
#include "resource_traps.h"
#include "system_traps.h"
#include "time_traps.h"
#include "timezone_ref.h"
#include "url_ref.h"
#include "work_queue_ref.h"

std::array<lyric_runtime::NativeTrap,35> kStdTraps = {{
    {std_process_get_program_id, "STD_PROCESS_GET_PROGRAM_ID", 0},
    {std_process_get_program_main, "STD_PROCESS_GET_PROGRAM_MAIN", 0},
    {std_process_get_argument, "STD_PROCESS_GET_ARGUMENT", 0},
    {std_process_num_arguments, "STD_PROCESS_NUM_ARGUMENTS", 0},
    {std_resource_exists, "STD_RESOURCE_EXISTS", 0},
    {std_resource_load, "STD_RESOURCE_LOAD", 0},
    {std_system_future_alloc, "STD_SYSTEM_FUTURE_ALLOC", 0},
    {std_system_future_ctor, "STD_SYSTEM_FUTURE_CTOR", 0},
    {std_system_future_complete, "STD_SYSTEM_FUTURE_COMPLETE", 0},
    {std_system_future_reject, "STD_SYSTEM_FUTURE_REJECT", 0},
    {std_system_future_then, "STD_SYSTEM_FUTURE_THEN", 0},
    {std_port_alloc, "STD_SYSTEM_PORT_ALLOC", 0},
    {std_port_receive, "STD_SYSTEM_PORT_RECEIVE", 0},
    {std_port_send, "STD_SYSTEM_PORT_SEND", 0},
    {std_system_work_queue_alloc, "STD_SYSTEM_WORK_QUEUE_ALLOC", 0},
    {std_system_work_queue_pop, "STD_SYSTEM_WORK_QUEUE_POP", 0},
    {std_system_work_queue_push, "STD_SYSTEM_WORK_QUEUE_PUSH", 0},
    {nullptr, "STD_SYSTEM_ACQUIRE", 0},
    {std_system_await, "STD_SYSTEM_AWAIT", 0},
    {std_system_get_result, "STD_SYSTEM_GET_RESULT", 0},
    {std_system_sleep, "STD_SYSTEM_SLEEP", 0},
    {std_system_spawn, "STD_SYSTEM_SPAWN", 0},
    {std_url_alloc, "STD_URL_URL_ALLOC", 0},
    {std_url_equality_equals, "STD_URL_EQUALITY_EQUALS", 0},
    {std_url_to_string, "STD_URL_URL_TO_STRING", 0},
    {std_url_parse_url, "STD_PARSE_URL", 0},
    {std_time_datetime_alloc, "STD_TIME_DATETIME_ALLOC", 0},
    {std_time_datetime_ctor, "STD_TIME_DATETIME_CTOR", 0},
    {std_time_instant_alloc, "STD_TIME_INSTANT_ALLOC", 0},
    {std_time_instant_ctor, "STD_TIME_INSTANT_CTOR", 0},
    {std_time_instant_to_epoch_millis, "STD_TIME_INSTANT_TO_EPOCH_MILLIS", 0},
    {std_time_timezone_alloc, "STD_TIME_TIMEZONE_ALLOC", 0},
    {std_time_timezone_ctor, "STD_TIME_TIMEZONE_CTOR", 0},
    {std_time_now, "STD_TIME_NOW", 0},
    {std_time_parse_timezone, "STD_TIME_PARSE_TIMEZONE", 0},
}};

class NativeStd : public lyric_runtime::NativeInterface {

public:
    NativeStd() = default;
    bool load(lyric_runtime::BytecodeSegment *segment) const override;
    void unload(lyric_runtime::BytecodeSegment *segment) const override;
    const lyric_runtime::NativeTrap *getTrap(uint32_t index) const override;
    uint32_t numTraps() const override;
};

const lyric_runtime::NativeTrap *
NativeStd::getTrap(uint32_t index) const
{
    if (kStdTraps.size() <= index)
        return nullptr;
    return &kStdTraps.at(index);
}

bool
NativeStd::load(lyric_runtime::BytecodeSegment *segment) const
{
    return true;
}

void
NativeStd::unload(lyric_runtime::BytecodeSegment *segment) const
{
}

uint32_t
NativeStd::numTraps() const
{
    return kStdTraps.size();
}

static const NativeStd iface;

const lyric_runtime::NativeInterface *native_init()
{
    return &iface;
}
