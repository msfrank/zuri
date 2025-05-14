/* SPDX-License-Identifier: BSD-3-Clause */

#include <vector>

#include <lyric_runtime/native_interface.h>
#include <tempo_utils/log_stream.h>

#include "log_traps.h"

std::array<lyric_runtime::NativeTrap,46> kStdLogTraps = {{
    {std_log_log, "STD_LOG_LOG", 0},
}};

class NativeStdLog : public lyric_runtime::NativeInterface {

public:
    NativeStdLog() = default;
    bool load(lyric_runtime::BytecodeSegment *segment) const override;
    void unload(lyric_runtime::BytecodeSegment *segment) const override;
    const lyric_runtime::NativeTrap *getTrap(uint32_t index) const override;
    tu_uint32 numTraps() const override;
};

const lyric_runtime::NativeTrap *
NativeStdLog::getTrap(uint32_t index) const
{
    if (kStdLogTraps.size() <= index)
        return nullptr;
    return &kStdLogTraps.at(index);
}

bool
NativeStdLog::load(lyric_runtime::BytecodeSegment *segment) const
{
    return true;
}

void
NativeStdLog::unload(lyric_runtime::BytecodeSegment *segment) const
{
}

uint32_t
NativeStdLog::numTraps() const
{
    return kStdLogTraps.size();
}

static const NativeStdLog iface;

void *std_log_init()
{
    return (void *) &iface;
}
