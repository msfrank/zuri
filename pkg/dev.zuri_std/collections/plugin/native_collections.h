/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_COLLECTIONS_NATIVE_COLLECTIONS_H
#define ZURI_STD_COLLECTIONS_NATIVE_COLLECTIONS_H

#include <lyric_runtime/native_interface.h>

class NativeStdCollections : public lyric_runtime::NativeInterface {

public:
    NativeStdCollections() = default;
    bool load(lyric_runtime::BytecodeSegment *segment) const override;
    void unload() const override;
    lyric_runtime::NativeFunc getTrap(uint32_t index) const override;
    uint32_t numTraps() const override;
};

#if defined(TARGET_OS_LINUX) || defined(TARGET_OS_MAC)

extern "C" const lyric_runtime::NativeInterface *native_init();

#elif defined(TARGET_OS_WINDOWS)

__declspec(dllexport) const lyric_runtime::NativeInterface *native_init();

#endif

#endif // ZURI_STD_COLLECTIONS_NATIVE_COLLECTIONS_H
