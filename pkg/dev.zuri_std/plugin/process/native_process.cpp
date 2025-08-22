/* SPDX-License-Identifier: BSD-3-Clause */

#include <tempo_utils/log_stream.h>

#include "native_process.h"
#include "process_traps.h"

std::array<lyric_runtime::NativeTrap,4> kStdProcessTraps = {{
    {std_process_get_program_id, "STD_PROCESS_GET_PROGRAM_ID", 0},
    {std_process_get_program_main, "STD_PROCESS_GET_PROGRAM_MAIN", 0},
    {std_process_get_argument, "STD_PROCESS_GET_ARGUMENT", 0},
    {std_process_num_arguments, "STD_PROCESS_NUM_ARGUMENTS", 0},
}};

class NativeStdProcess : public lyric_runtime::NativeInterface {

public:
    NativeStdProcess() = default;
    bool load(lyric_runtime::BytecodeSegment *segment) const override;
    void unload(lyric_runtime::BytecodeSegment *segment) const override;
    const lyric_runtime::NativeTrap *getTrap(uint32_t index) const override;
    uint32_t numTraps() const override;
};

const lyric_runtime::NativeTrap *
NativeStdProcess::getTrap(uint32_t index) const
{
    if (kStdProcessTraps.size() <= index)
        return nullptr;
    return &kStdProcessTraps.at(index);
}

bool
NativeStdProcess::load(lyric_runtime::BytecodeSegment *segment) const
{
    return true;
}

void
NativeStdProcess::unload(lyric_runtime::BytecodeSegment *segment) const
{
}

uint32_t
NativeStdProcess::numTraps() const
{
    return kStdProcessTraps.size();
}

static const NativeStdProcess iface;

const lyric_runtime::NativeInterface *native_init()
{
    return &iface;
}
