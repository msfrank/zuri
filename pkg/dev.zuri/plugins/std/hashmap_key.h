/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_COLLECTIONS_HASHMAP_KEY_H
#define ZURI_STD_COLLECTIONS_HASHMAP_KEY_H

#include <lyric_runtime/operand.h>

struct HashMapKey {
    lyric_runtime::Operand cell;
};

template <typename H>
H AbslHashValue(H state, const HashMapKey &key) {
    const auto &cell = key.cell;
    cell.hashEquality(absl::HashState::Create(&state));
    return std::move(state);
}

#endif // ZURI_STD_COLLECTIONS_HASHMAP_KEY_H