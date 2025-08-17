/* SPDX-License-Identifier: AGPL-3.0-or-later */

#ifndef ZURI_PKG_ZURI_PKG_H
#define ZURI_PKG_ZURI_PKG_H

#include <tempo_utils/status.h>

namespace zuri_pkg {
    tempo_utils::Status zuri_pkg(int argc, const char *argv[]);
}

#endif // ZURI_PKG_ZURI_PKG_H