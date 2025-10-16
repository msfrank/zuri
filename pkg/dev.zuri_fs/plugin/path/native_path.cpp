/* SPDX-License-Identifier: BSD-3-Clause */

#include <tempo_utils/log_stream.h>

#include "native_path.h"
#include "path_ref.h"

std::array<lyric_runtime::NativeTrap,10> kFsPathTraps = {{
    {fs_path_alloc, "FS_PATH_ALLOC", 0},
    {fs_path_ctor, "FS_PATH_CTOR", 0},
    {fs_path_parent, "FS_PATH_PARENT", 0},
    {fs_path_resolve, "FS_PATH_RESOLVE", 0},
    {fs_path_file_name, "FS_PATH_FILE_NAME", 0},
    {fs_path_file_stem, "FS_PATH_FILE_STEM", 0},
    {fs_path_file_extension, "FS_PATH_FILE_EXTENSION", 0},
    {fs_path_is_absolute, "FS_PATH_IS_ABSOLUTE", 0},
    {fs_path_is_relative, "FS_PATH_IS_RELATIVE", 0},
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
