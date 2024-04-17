/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_TEXT_LIB_TYPES_H
#define ZURI_STD_TEXT_LIB_TYPES_H

#include <cstdint>

enum class StdTextTrap : uint32_t {
    TEXT_ALLOC,
    TEXT_CTOR,
    TEXT_LENGTH,
    TEXT_AT,
    TEXT_ITER,
    LAST_,
};

#endif // ZURI_STD_TEXT_LIB_TYPES_H
