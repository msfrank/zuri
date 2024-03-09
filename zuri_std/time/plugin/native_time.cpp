/* SPDX-License-Identifier: BSD-3-Clause */

#include <tempo_utils/log_stream.h>
#include <zuri_std_time/lib_types.h>

#include "datetime_ref.h"
#include "instant_ref.h"
#include "native_time.h"
#include "timezone_ref.h"
#include "time_traps.h"

lyric_runtime::NativeFunc
NativeStdTime::getTrap(uint32_t index) const
{
    if (index >= static_cast<uint32_t>(StdTimeTrap::LAST_))
        return nullptr;
    auto trapFunction = static_cast<StdTimeTrap>(index);
    switch (trapFunction) {
        case StdTimeTrap::DATETIME_ALLOC:
            return datetime_alloc;
        case StdTimeTrap::DATETIME_CTOR:
            return datetime_ctor;
        case StdTimeTrap::INSTANT_ALLOC:
            return instant_alloc;
        case StdTimeTrap::INSTANT_CTOR:
            return instant_ctor;
        case StdTimeTrap::INSTANT_TO_EPOCH_MILLIS:
            return instant_to_epoch_millis;
        case StdTimeTrap::TIMEZONE_ALLOC:
            return timezone_alloc;
        case StdTimeTrap::TIMEZONE_CTOR:
            return timezone_ctor;
        case StdTimeTrap::NOW:
            return std_time_now;
        case StdTimeTrap::PARSE_TIMEZONE:
            return std_time_parse_timezone;

        case StdTimeTrap::LAST_:
            break;
    }
    TU_UNREACHABLE();
}

uint32_t
NativeStdTime::numTraps() const
{
    return static_cast<uint32_t>(StdTimeTrap::LAST_);
}

static const NativeStdTime iface;

const lyric_runtime::NativeInterface *native_init()
{
    return &iface;
}
