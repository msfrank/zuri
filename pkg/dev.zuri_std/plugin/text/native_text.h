/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_TEXT_NATIVE_TEXT_H
#define ZURI_STD_TEXT_NATIVE_TEXT_H

#include <boost/predef.h>

#include <lyric_runtime/native_interface.h>

#if defined(TARGET_OS_LINUX) || defined(TARGET_OS_MAC)

extern "C" const lyric_runtime::NativeInterface *native_init();

#elif defined(TARGET_OS_WINDOWS)

__declspec(dllexport) const lyric_runtime::NativeInterface *native_init();

#else

#error "unsupported operating system"

#endif

#endif // ZURI_STD_TEXT_NATIVE_TEXT_H
