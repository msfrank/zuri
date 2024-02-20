/* SPDX-License-Identifier: BSD-3-Clause */

#include <tempo_utils/log_stream.h>
#include <zuri_std_text/lib_types.h>

#include "native_text.h"
#include "text_ref.h"

lyric_runtime::NativeFunc
NativeStdText::getTrap(uint32_t index) const
{
    if (index >= static_cast<uint32_t>(StdTextTrap::LAST_))
        return nullptr;
    auto trapFunction = static_cast<StdTextTrap>(index);
    switch (trapFunction) {
        case StdTextTrap::TEXT_ALLOC:
            return text_alloc;
        case StdTextTrap::TEXT_CTOR:
            return text_ctor;
        case StdTextTrap::TEXT_SIZE:
            return text_size;
        case StdTextTrap::TEXT_AT:
            return text_at;
        case StdTextTrap::TEXT_ITER:
            return nullptr;

        case StdTextTrap::LAST_:
            break;
    }
    TU_UNREACHABLE();
}

uint32_t
NativeStdText::numTraps() const
{
    return static_cast<uint32_t>(StdTextTrap::LAST_);
}

static const NativeStdText iface;

const lyric_runtime::NativeInterface *native_init()
{
    return &iface;
}
