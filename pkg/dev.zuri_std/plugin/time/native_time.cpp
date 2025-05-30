/* SPDX-License-Identifier: BSD-3-Clause */

#include <tempo_utils/log_stream.h>

#include "datetime_ref.h"
#include "instant_ref.h"
#include "native_time.h"
#include "timezone_ref.h"
#include "time_traps.h"

std::array<lyric_runtime::NativeTrap,9> kStdTimeTraps = {{
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

class NativeStdTime : public lyric_runtime::NativeInterface {

public:
    NativeStdTime() = default;
    bool load(lyric_runtime::BytecodeSegment *segment) const override;
    void unload(lyric_runtime::BytecodeSegment *segment) const override;
    const lyric_runtime::NativeTrap *getTrap(uint32_t index) const override;
    uint32_t numTraps() const override;
};

const lyric_runtime::NativeTrap *
NativeStdTime::getTrap(uint32_t index) const
{
    if (kStdTimeTraps.size() <= index)
        return nullptr;
    return &kStdTimeTraps.at(index);
}

bool
NativeStdTime::load(lyric_runtime::BytecodeSegment *segment) const
{
    return true;
}

void
NativeStdTime::unload(lyric_runtime::BytecodeSegment *segment) const
{
}

uint32_t
NativeStdTime::numTraps() const
{
    return kStdTimeTraps.size();
}

static const NativeStdTime iface;

const lyric_runtime::NativeInterface *native_init()
{
    return &iface;
}
