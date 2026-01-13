/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_SYSTEM_NATIVE_SYSTEM_H
#define ZURI_STD_SYSTEM_NATIVE_SYSTEM_H

#include <boost/predef.h>

#if defined(BOOST_OS_LINUX) || defined(BOOST_OS_MACOS)

extern "C" const lyric_runtime::NativeInterface *native_init();

#elif defined(BOOST_OS_WINDOWS)

__declspec(dllexport) const lyric_runtime::NativeInterface *native_init();

#else

#error "unsupported operating system"

#endif

#endif // ZURI_STD_SYSTEM_NATIVE_SYSTEM_H
