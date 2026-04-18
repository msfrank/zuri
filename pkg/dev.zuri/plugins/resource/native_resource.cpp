/* SPDX-License-Identifier: BSD-3-Clause */

#include <tempo_utils/log_stream.h>

#include "native_resource.h"
#include "resource_traps.h"
#include "url_ref.h"

std::array<lyric_runtime::NativeTrap,6> kStdResourceTraps = {{
    {std_resource_exists, "STD_RESOURCE_EXISTS", 0},
    {std_resource_load, "STD_RESOURCE_LOAD", 0},
    {std_url_alloc, "STD_URL_URL_ALLOC", 0},
    {std_url_equality_equals, "STD_URL_EQUALITY_EQUALS", 0},
    {std_url_to_string, "STD_URL_URL_TO_STRING", 0},
    {std_parse_url, "STD_PARSE_URL", 0},
}};

class NativeStdResource : public lyric_runtime::NativeInterface {

public:
    NativeStdResource() = default;
    bool load(lyric_runtime::BytecodeSegment *segment) const override;
    void unload(lyric_runtime::BytecodeSegment *segment) const override;
    const lyric_runtime::NativeTrap *getTrap(uint32_t index) const override;
    tu_uint32 numTraps() const override;
};

const lyric_runtime::NativeTrap *
NativeStdResource::getTrap(uint32_t index) const
{
    if (kStdResourceTraps.size() <= index)
        return nullptr;
    return &kStdResourceTraps.at(index);
}

bool
NativeStdResource::load(lyric_runtime::BytecodeSegment *segment) const
{
    return true;
}

void
NativeStdResource::unload(lyric_runtime::BytecodeSegment *segment) const
{
}

uint32_t
NativeStdResource::numTraps() const
{
    return kStdResourceTraps.size();
}

static const NativeStdResource iface;

const lyric_runtime::NativeInterface *native_init()
{
    return &iface;
}
