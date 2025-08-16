/* SPDX-License-Identifier: AGPL-3.0-or-later */

#ifndef ZURI_BUILD_ZURI_BUILD_H
#define ZURI_BUILD_ZURI_BUILD_H

#include <tempo_utils/status.h>

namespace zuri_build {
    tempo_utils::Status zuri_build(int argc, const char *argv[]);
}

#endif // ZURI_BUILD_ZURI_BUILD_H