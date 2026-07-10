/* SPDX-License-Identifier: BSD-3-Clause */

#include "conn_traps.h"
#include "connection_ref.h"
#include "datetime_ref.h"
#include "future_ref.h"
#include "hashmap_traps.h"
#include "instant_ref.h"
#include "native_std.h"
#include "port_ref.h"
#include "process_traps.h"
#include "resource_traps.h"
#include "system_traps.h"
#include "time_traps.h"
#include "timezone_ref.h"
#include "treemap_traps.h"
#include "treeset_traps.h"
#include "url_ref.h"
#include "vector_traps.h"
#include "work_queue_ref.h"

std::array<lyric_runtime::NativeTrap,88> kStdTraps = {{
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
    {std_system_work_queue_alloc, "STD_SYSTEM_WORK_QUEUE_ALLOC", 0},
    {std_system_work_queue_pop, "STD_SYSTEM_WORK_QUEUE_POP", 0},
    {std_system_work_queue_push, "STD_SYSTEM_WORK_QUEUE_PUSH", 0},
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
    {hashmap_alloc, "STD_COLLECTIONS_HASHMAP_ALLOC", 0},
    {hashmap_ctor, "STD_COLLECTIONS_HASHMAP_CTOR", 0},
    {hashmap_size, "STD_COLLECTIONS_HASHMAP_SIZE", 0},
    {hashmap_contains, "STD_COLLECTIONS_HASHMAP_CONTAINS", 0},
    {hashmap_get, "STD_COLLECTIONS_HASHMAP_GET", 0},
    {hashmap_put, "STD_COLLECTIONS_HASHMAP_PUT", 0},
    {hashmap_remove, "STD_COLLECTIONS_HASHMAP_REMOVE", 0},
    {hashmap_clear, "STD_COLLECTIONS_HASHMAP_CLEAR", 0},
    {hashmap_iterate, "STD_COLLECTIONS_HASHMAP_ITERABLE_ITERATE", 0},
    {hashmap_iterator_alloc, "STD_COLLECTIONS_HASHMAP_ITERATOR_ALLOC", 0},
    {hashmap_iterator_valid, "STD_COLLECTIONS_HASHMAP_ITERATOR_VALID", 0},
    {hashmap_iterator_get_key, "STD_COLLECTIONS_HASHMAP_ITERATOR_GET_KEY", 0},
    {hashmap_iterator_get_value, "STD_COLLECTIONS_HASHMAP_ITERATOR_GET_VALUE", 0},
    {hashmap_iterator_next, "STD_COLLECTIONS_HASHMAP_ITERATOR_NEXT", 0},
    {treemap_alloc, "STD_COLLECTIONS_TREEMAP_ALLOC", 0},
    {treemap_ctor, "STD_COLLECTIONS_TREEMAP_CTOR", 0},
    {treemap_size, "STD_COLLECTIONS_TREEMAP_SIZE", 0},
    {treemap_contains, "STD_COLLECTIONS_TREEMAP_CONTAINS", 0},
    {treemap_get, "STD_COLLECTIONS_TREEMAP_GET", 0},
    {treemap_put, "STD_COLLECTIONS_TREEMAP_PUT", 0},
    {treemap_remove, "STD_COLLECTIONS_TREEMAP_REMOVE", 0},
    {treemap_clear, "STD_COLLECTIONS_TREEMAP_CLEAR", 0},
    {treemap_iterate, "STD_COLLECTIONS_TREEMAP_ITERABLE_ITERATE", 0},
    {treemap_iterator_alloc, "STD_COLLECTIONS_TREEMAP_ITERATOR_ALLOC", 0},
    {treemap_iterator_valid, "STD_COLLECTIONS_TREEMAP_ITERATOR_VALID", 0},
    {treemap_iterator_get_key, "STD_COLLECTIONS_TREEMAP_ITERATOR_GET_KEY", 0},
    {treemap_iterator_get_value, "STD_COLLECTIONS_TREEMAP_ITERATOR_GET_VALUE", 0},
    {treemap_iterator_next, "STD_COLLECTIONS_TREEMAP_ITERATOR_NEXT", 0},
    {treeset_alloc, "STD_COLLECTIONS_TREESET_ALLOC", 0},
    {treeset_ctor, "STD_COLLECTIONS_TREESET_CTOR", 0},
    {treeset_size, "STD_COLLECTIONS_TREESET_SIZE", 0},
    {treeset_contains, "STD_COLLECTIONS_TREESET_CONTAINS", 0},
    {treeset_add, "STD_COLLECTIONS_TREESET_ADD", 0},
    {treeset_remove, "STD_COLLECTIONS_TREESET_REMOVE", 0},
    {treeset_replace, "STD_COLLECTIONS_TREESET_REPLACE", 0},
    {treeset_clear, "STD_COLLECTIONS_TREESET_CLEAR", 0},
    {treeset_iterate, "STD_COLLECTIONS_TREESET_ITERABLE_ITERATE", 0},
    {treeset_iterator_alloc, "STD_COLLECTIONS_TREESET_ITERATOR_ALLOC", 0},
    {treeset_iterator_next, "STD_COLLECTIONS_TREESET_ITERATOR_NEXT", 0},
    {treeset_iterator_valid, "STD_COLLECTIONS_TREESET_ITERATOR_VALID", 0},
    {vector_alloc, "STD_COLLECTIONS_VECTOR_ALLOC", 0},
    {vector_ctor, "STD_COLLECTIONS_VECTOR_CTOR", 0},
    {vector_size, "STD_COLLECTIONS_VECTOR_SIZE", 0},
    {vector_at, "STD_COLLECTIONS_VECTOR_AT", 0},
    {vector_append, "STD_COLLECTIONS_VECTOR_APPEND", 0},
    {vector_insert, "STD_COLLECTIONS_VECTOR_INSERT", 0},
    {vector_replace, "STD_COLLECTIONS_VECTOR_REPLACE", 0},
    {vector_remove, "STD_COLLECTIONS_VECTOR_REMOVE", 0},
    {vector_clear, "STD_COLLECTIONS_VECTOR_CLEAR", 0},
    {vector_iterate, "STD_COLLECTIONS_VECTOR_ITERABLE_ITERATE", 0},
    {vector_iterator_alloc, "STD_COLLECTIONS_VECTOR_ITERATOR_ALLOC", 0},
    {vector_iterator_next, "STD_COLLECTIONS_VECTOR_ITERATOR_NEXT", 0},
    {vector_iterator_valid, "STD_COLLECTIONS_VECTOR_ITERATOR_VALID", 0},
    {std_conn_make_connection, "STD_CONN_MAKE_CONNECTION", 0},
    {std_connection_alloc, "STD_CONN_CONNECTION_ALLOC", 0},
    {std_connection_send, "STD_CONN_CONNECTION_SEND", 0},
    {std_connection_receive, "STD_CONN_CONNECTION_RECEIVE", 0},
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
