/* SPDX-License-Identifier: AGPL-3.0-or-later */

#ifndef ZURI_RUN_ZURI_RUN_H
#define ZURI_RUN_ZURI_RUN_H

#include <tempo_utils/status.h>

namespace zuri_run {
    tempo_utils::Status zuri_run(int argc, const char *argv[]);
}

#endif // ZURI_RUN_ZURI_RUN_H