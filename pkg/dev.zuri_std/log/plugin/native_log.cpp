/* SPDX-License-Identifier: BSD-3-Clause */

#include <vector>

#include <tempo_utils/log_stream.h>
#include <zuri_std_log/lib_types.h>

#include "log_traps.h"
#include "native_log.h"

lyric_runtime::NativeFunc
NativeStdLog::getTrap(uint32_t index) const
{
    if (index >= static_cast<uint32_t>(StdLogTrap::LAST_))
        return nullptr;
    auto trapFunction = static_cast<StdLogTrap>(index);
    switch (trapFunction) {
        case StdLogTrap::LOG:
            return std_log_log;

        case StdLogTrap::LAST_:
            break;
    }
    TU_UNREACHABLE();
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
    return static_cast<uint32_t>(StdLogTrap::LAST_);
}

static const NativeStdLog iface;

const lyric_runtime::NativeInterface *native_init()
{
    return &iface;
}
