/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_COLLECTIONS_HASHMAP_KEY_H
#define ZURI_STD_COLLECTIONS_HASHMAP_KEY_H

#include <absl/hash/hash.h>

#include <lyric_runtime/data_cell.h>

struct HashMapKey {
    lyric_runtime::DataCell cell;
};

template <typename H>
H AbslHashValue(H state, const HashMapKey &key) {
    const auto &cell = key.cell;
    switch (cell.type) {
        case lyric_runtime::DataCellType::INVALID:
            return H::combine(std::move(state), 0);
        case lyric_runtime::DataCellType::NIL:
            return H::combine(std::move(state), 0);
        case lyric_runtime::DataCellType::PRESENT:
            return H::combine(std::move(state), 1);
        case lyric_runtime::DataCellType::BOOL:
            return H::combine(std::move(state), cell.data.b);
        case lyric_runtime::DataCellType::CHAR32:
            return H::combine(std::move(state), cell.data.chr);
        case lyric_runtime::DataCellType::I64:
            return H::combine(std::move(state), cell.data.i64);
        case lyric_runtime::DataCellType::DBL:
            return H::combine(std::move(state), cell.data.dbl);
        case lyric_runtime::DataCellType::UTF8:
            return H::combine_contiguous(std::move(state), cell.data.utf8.data, cell.data.utf8.size);
        case lyric_runtime::DataCellType::TYPE:
        case lyric_runtime::DataCellType::CONCEPT:
        case lyric_runtime::DataCellType::FIELD:
        case lyric_runtime::DataCellType::ENUM:
        case lyric_runtime::DataCellType::CALL:
        case lyric_runtime::DataCellType::CLASS:
        case lyric_runtime::DataCellType::STRUCT:
        case lyric_runtime::DataCellType::INSTANCE:
        case lyric_runtime::DataCellType::ACTION:
        case lyric_runtime::DataCellType::EXISTENTIAL:
        case lyric_runtime::DataCellType::NAMESPACE:
            return H::combine(std::move(state), cell.data.descriptor.assembly, cell.data.descriptor.value);
        case lyric_runtime::DataCellType::REF:
            cell.data.ref->hashValue(absl::HashState::Create(&state));
            return std::move(state);
    }
}

#endif // ZURI_STD_COLLECTIONS_HASHMAP_KEY_H