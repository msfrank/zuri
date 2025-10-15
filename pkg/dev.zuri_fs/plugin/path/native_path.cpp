/* SPDX-License-Identifier: BSD-3-Clause */

#include <tempo_utils/log_stream.h>

#include "native_path.h"
#include "path_ref.h"

std::array<lyric_runtime::NativeTrap,3> kFsPathTraps = {{
    {fs_path_alloc, "FS_PATH_ALLOC", 0},
    {fs_path_ctor, "FS_PATH_CTOR", 0},
    {fs_path_to_string, "FS_PATH_TO_STRING", 0},
}};

class NativeFsPath : public lyric_runtime::NativeInterface {

public:
    NativeFsPath() = default;
    bool load(lyric_runtime::BytecodeSegment *segment) const override;
    void unload(lyric_runtime::BytecodeSegment *segment) const override;
    const lyric_runtime::NativeTrap *getTrap(uint32_t index) const override;
    uint32_t numTraps() const override;
};

const lyric_runtime::NativeTrap *
NativeFsPath::getTrap(uint32_t index) const
{
    if (kFsPathTraps.size() <= index)
        return nullptr;
    return &kFsPathTraps.at(index);
}

bool
NativeFsPath::load(lyric_runtime::BytecodeSegment *segment) const
{
    return true;
}

void
NativeFsPath::unload(lyric_runtime::BytecodeSegment *segment) const
{
}

uint32_t
NativeFsPath::numTraps() const
{
    return kFsPathTraps.size();
}

static const NativeFsPath iface;

const lyric_runtime::NativeInterface *native_init()
{
    return &iface;
}
