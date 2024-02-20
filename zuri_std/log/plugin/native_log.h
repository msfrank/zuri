/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_LOG_NATIVE_LOG_H
#define ZURI_STD_LOG_NATIVE_LOG_H

#include <lyric_runtime/native_interface.h>

class NativeStdLog : public lyric_runtime::NativeInterface {

public:
    NativeStdLog() = default;
    lyric_runtime::NativeFunc getTrap(uint32_t index) const override;
    uint32_t numTraps() const override;
};

#if defined(TARGET_OS_LINUX) || defined(TARGET_OS_MAC)

extern "C" const lyric_runtime::NativeInterface *native_init();

#elif defined(TARGET_OS_WINDOWS)

__declspec(dllexport) const lyric_runtime::NativeInterface *native_init();

#endif

#endif // ZURI_STD_LOG_NATIVE_LOG_H
