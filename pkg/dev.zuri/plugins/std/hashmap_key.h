/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_COLLECTIONS_HASHMAP_KEY_H
#define ZURI_STD_COLLECTIONS_HASHMAP_KEY_H

#include <absl/hash/hash.h>

#include <lyric_runtime/bytes_ref.h>
#include <lyric_runtime/data_cell.h>
#include <lyric_runtime/namespace_ref.h>
#include <lyric_runtime/protocol_ref.h>
#include <lyric_runtime/rest_ref.h>
#include <lyric_runtime/status_ref.h>
#include <lyric_runtime/string_ref.h>

struct HashMapKey {
    lyric_runtime::DataCell cell;
};

template <typename H>
H AbslHashValue(H state, const HashMapKey &key) {
    const auto &cell = key.cell;
    switch (cell.type) {
        case lyric_runtime::DataCellType::Invalid:
        case lyric_runtime::DataCellType::Undef:
            return H::combine(std::move(state), 0);
        case lyric_runtime::DataCellType::Nil:
            return H::combine(std::move(state), 1);
        case lyric_runtime::DataCellType::Bool:
            return H::combine(std::move(state), cell.data.b);
        case lyric_runtime::DataCellType::Char32:
            return H::combine(std::move(state), cell.data.chr);
        case lyric_runtime::DataCellType::Int64:
            return H::combine(std::move(state), cell.data.i64);
        case lyric_runtime::DataCellType::Float64:
            return H::combine(std::move(state), cell.data.dbl);
        case lyric_runtime::DataCellType::Ref:
            cell.data.ref->hashValue(absl::HashState::Create(&state));
            return std::move(state);
        case lyric_runtime::DataCellType::String:
            cell.data.str->hashValue(absl::HashState::Create(&state));
            return std::move(state);
        case lyric_runtime::DataCellType::Bytes:
            cell.data.bytes->hashValue(absl::HashState::Create(&state));
            return std::move(state);
        case lyric_runtime::DataCellType::Status:
            cell.data.status->hashValue(absl::HashState::Create(&state));
            return std::move(state);
        case lyric_runtime::DataCellType::Namespace:
            cell.data.ns->hashValue(absl::HashState::Create(&state));
            return std::move(state);
        case lyric_runtime::DataCellType::Protocol:
            cell.data.protocol->hashValue(absl::HashState::Create(&state));
            return std::move(state);
        case lyric_runtime::DataCellType::Rest:
            cell.data.rest->hashValue(absl::HashState::Create(&state));
            return std::move(state);

        case lyric_runtime::DataCellType::Type:
            return H::combine(std::move(state),
                cell.data.type->getSegmentIndex(),
                cell.data.type->getDescriptorIndex());

        case lyric_runtime::DataCellType::Descriptor:
            return H::combine(std::move(state),
                cell.data.descriptor->getSegmentIndex(),
                cell.data.descriptor->getLinkageSection(),
                cell.data.descriptor->getDescriptorIndex());
    }
}

#endif // ZURI_STD_COLLECTIONS_HASHMAP_KEY_H