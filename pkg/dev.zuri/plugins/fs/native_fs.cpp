/* SPDX-License-Identifier: BSD-3-Clause */

#include <tempo_utils/log_stream.h>

#include "file_ref.h"
#include "native_fs.h"
#include "path_ref.h"

std::array<lyric_runtime::NativeTrap,18> kFsTraps = {{
    {fs_file_alloc, "FS_FILE_ALLOC", 0},
    {fs_file_ctor, "FS_FILE_CTOR", 0},
    {fs_file_create, "FS_FILE_CREATE", 0},
    {fs_file_open, "FS_FILE_OPEN", 0},
    {fs_file_open_or_create, "FS_FILE_OPEN_OR_CREATE", 0},
    {fs_file_read, "FS_FILE_READ", 0},
    {fs_file_write, "FS_FILE_WRITE", 0},
    {fs_file_close, "FS_FILE_CLOSE", 0},
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

class NativeFs : public lyric_runtime::NativeInterface {

public:
    NativeFs() = default;
    bool load(lyric_runtime::BytecodeSegment *segment) const override;
    void unload(lyric_runtime::BytecodeSegment *segment) const override;
    const lyric_runtime::NativeTrap *getTrap(uint32_t index) const override;
    uint32_t numTraps() const override;
};

const lyric_runtime::NativeTrap *
NativeFs::getTrap(uint32_t index) const
{
    if (kFsTraps.size() <= index)
        return nullptr;
    return &kFsTraps.at(index);
}

bool
NativeFs::load(lyric_runtime::BytecodeSegment *segment) const
{
    return true;
}

void
NativeFs::unload(lyric_runtime::BytecodeSegment *segment) const
{
}

uint32_t
NativeFs::numTraps() const
{
    return kFsTraps.size();
}

static const NativeFs iface;

const lyric_runtime::NativeInterface *native_init()
{
    return &iface;
}
