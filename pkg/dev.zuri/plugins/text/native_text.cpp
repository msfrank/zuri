/* SPDX-License-Identifier: BSD-3-Clause */

#include <tempo_utils/log_stream.h>

#include "native_text.h"
#include "text_ref.h"

std::array<lyric_runtime::NativeTrap,4> kStdTextTraps = {{
    {std_text_text_alloc, "STD_TEXT_TEXT_ALLOC", 0},
    {std_text_text_ctor, "STD_TEXT_TEXT_CTOR", 0},
    {std_text_text_length, "STD_TEXT_TEXT_LENGTH", 0},
    {std_text_text_at, "STD_TEXT_TEXT_AT", 0},
}};

class NativeStdText : public lyric_runtime::NativeInterface {

public:
    NativeStdText() = default;
    bool load(lyric_runtime::BytecodeSegment *segment) const override;
    void unload(lyric_runtime::BytecodeSegment *segment) const override;
    const lyric_runtime::NativeTrap *getTrap(uint32_t index) const override;
    uint32_t numTraps() const override;
};

const lyric_runtime::NativeTrap *
NativeStdText::getTrap(uint32_t index) const
{
    if (kStdTextTraps.size() <= index)
        return nullptr;
    return &kStdTextTraps.at(index);
}

bool
NativeStdText::load(lyric_runtime::BytecodeSegment *segment) const
{
    return true;
}

void
NativeStdText::unload(lyric_runtime::BytecodeSegment *segment) const
{
}

uint32_t
NativeStdText::numTraps() const
{
    return kStdTextTraps.size();
}

static const NativeStdText iface;

const lyric_runtime::NativeInterface *native_init()
{
    return &iface;
}
