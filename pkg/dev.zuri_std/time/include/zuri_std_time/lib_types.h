/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_TIME_LIB_TYPES_H
#define ZURI_STD_TIME_LIB_TYPES_H

#include <cstdint>

enum class StdTimeTrap : uint32_t {
    DATETIME_ALLOC,
    DATETIME_CTOR,
    INSTANT_ALLOC,
    INSTANT_CTOR,
    INSTANT_TO_EPOCH_MILLIS,
    NOW,
    PARSE_TIMEZONE,
    TIMEZONE_ALLOC,
    TIMEZONE_CTOR,
    LAST_,
};

#endif // ZURI_STD_TIME_LIB_TYPES_H
