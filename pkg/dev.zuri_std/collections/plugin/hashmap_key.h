/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_COLLECTIONS_HASHMAP_KEY_H
#define ZURI_STD_COLLECTIONS_HASHMAP_KEY_H

#include <absl/hash/hash.h>

#include <lyric_runtime/data_cell.h>
#include <lyric_runtime/string_ref.h>
#include <lyric_runtime/url_ref.h>

struct HashMapKey {
    lyric_runtime::DataCell cell;
};

template <typename H>
H AbslHashValue(H state, const HashMapKey &key) {
    const auto &cell = key.cell;
    switch (cell.type) {
        case lyric_runtime::DataCellType::INVALID:
        case lyric_runtime::DataCellType::NIL:
            return H::combine(std::move(state), 0);
        case lyric_runtime::DataCellType::UNDEF:
            return H::combine(std::move(state), 1);
        case lyric_runtime::DataCellType::BOOL:
            return H::combine(std::move(state), cell.data.b);
        case lyric_runtime::DataCellType::CHAR32:
            return H::combine(std::move(state), cell.data.chr);
        case lyric_runtime::DataCellType::I64:
            return H::combine(std::move(state), cell.data.i64);
        case lyric_runtime::DataCellType::DBL:
            return H::combine(std::move(state), cell.data.dbl);
        case lyric_runtime::DataCellType::REF:
            cell.data.ref->hashValue(absl::HashState::Create(&state));
            return std::move(state);
        case lyric_runtime::DataCellType::STRING:
            cell.data.str->hashValue(absl::HashState::Create(&state));
            return std::move(state);
        case lyric_runtime::DataCellType::URL:
            cell.data.url->hashValue(absl::HashState::Create(&state));
            return std::move(state);
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
    }
}

#endif // ZURI_STD_COLLECTIONS_HASHMAP_KEY_H