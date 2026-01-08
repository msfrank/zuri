/* SPDX-License-Identifier: AGPL-3.0-or-later */

#ifndef ZURI_ENV_ZURI_ENV_H
#define ZURI_ENV_ZURI_ENV_H

#include <tempo_utils/status.h>

namespace zuri_env {
    tempo_utils::Status zuri_env(int argc, const char *argv[]);
}

#endif // ZURI_ENV_ZURI_ENV_H