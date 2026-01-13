/* SPDX-License-Identifier: BSD-3-Clause */

#include <tempo_utils/log_stream.h>

#include "file_ref.h"
#include "native_file.h"

std::array<lyric_runtime::NativeTrap,8> kFsFileTraps = {{
    {fs_file_alloc, "FS_FILE_ALLOC", 0},
    {fs_file_ctor, "FS_FILE_CTOR", 0},
    {fs_file_create, "FS_FILE_CREATE", 0},
    {fs_file_open, "FS_FILE_OPEN", 0},
    {fs_file_open_or_create, "FS_FILE_OPEN_OR_CREATE", 0},
    {fs_file_read, "FS_FILE_READ", 0},
    {fs_file_write, "FS_FILE_WRITE", 0},
    {fs_file_close, "FS_FILE_CLOSE", 0},
}};

class NativeFsFile : public lyric_runtime::NativeInterface {

public:
    NativeFsFile() = default;
    bool load(lyric_runtime::BytecodeSegment *segment) const override;
    void unload(lyric_runtime::BytecodeSegment *segment) const override;
    const lyric_runtime::NativeTrap *getTrap(uint32_t index) const override;
    uint32_t numTraps() const override;
};

const lyric_runtime::NativeTrap *
NativeFsFile::getTrap(uint32_t index) const
{
    if (kFsFileTraps.size() <= index)
        return nullptr;
    return &kFsFileTraps.at(index);
}

bool
NativeFsFile::load(lyric_runtime::BytecodeSegment *segment) const
{
    return true;
}

void
NativeFsFile::unload(lyric_runtime::BytecodeSegment *segment) const
{
}

uint32_t
NativeFsFile::numTraps() const
{
    return kFsFileTraps.size();
}

static const NativeFsFile iface;

const lyric_runtime::NativeInterface *native_init()
{
    return &iface;
}
