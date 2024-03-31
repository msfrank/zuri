/* SPDX-License-Identifier: BSD-3-Clause */

#include <vector>

#include <tempo_utils/log_stream.h>
#include <zuri_std_collections/lib_types.h>

#include "hashmap_traps.h"
#include "native_collections.h"
#include "treemap_traps.h"
#include "treeset_traps.h"
#include "vector_traps.h"
#include "option_ref.h"

lyric_runtime::NativeFunc
NativeStdCollections::getTrap(uint32_t index) const
{
    if (index >= static_cast<uint32_t>(StdCollectionsTrap::LAST_))
        return nullptr;
    auto trapFunction = static_cast<StdCollectionsTrap>(index);
    switch (trapFunction) {
        case StdCollectionsTrap::HASHMAP_ALLOC:
            return hashmap_alloc;
        case StdCollectionsTrap::HASHMAP_CTOR:
            return hashmap_ctor;
        case StdCollectionsTrap::HASHMAP_SIZE:
            return hashmap_size;
        case StdCollectionsTrap::HASHMAP_CONTAINS:
            return hashmap_contains;
        case StdCollectionsTrap::HASHMAP_GET:
            return hashmap_get;
        case StdCollectionsTrap::HASHMAP_PUT:
            return hashmap_put;
        case StdCollectionsTrap::HASHMAP_REMOVE:
            return hashmap_remove;
        case StdCollectionsTrap::HASHMAP_CLEAR:
            return hashmap_clear;
        case StdCollectionsTrap::HASHMAP_ITER:
            return nullptr;
        case StdCollectionsTrap::OPTION_ALLOC:
            return option_alloc;
        case StdCollectionsTrap::OPTION_CTOR:
            return option_ctor;
        case StdCollectionsTrap::OPTION_GET:
            return option_get;
        case StdCollectionsTrap::OPTION_IS_EMPTY:
            return option_is_empty;
        case StdCollectionsTrap::TREEMAP_ALLOC:
            return treemap_alloc;
        case StdCollectionsTrap::TREEMAP_CTOR:
            return treemap_ctor;
        case StdCollectionsTrap::TREEMAP_SIZE:
            return treemap_size;
        case StdCollectionsTrap::TREEMAP_CONTAINS:
            return treemap_contains;
        case StdCollectionsTrap::TREEMAP_GET:
            return treemap_get;
        case StdCollectionsTrap::TREEMAP_PUT:
            return treemap_put;
        case StdCollectionsTrap::TREEMAP_REMOVE:
            return treemap_remove;
        case StdCollectionsTrap::TREEMAP_CLEAR:
            return treemap_clear;
        case StdCollectionsTrap::TREEMAP_ITER:
            return nullptr;
        case StdCollectionsTrap::TREESET_ALLOC:
            return treeset_alloc;
        case StdCollectionsTrap::TREESET_CTOR:
            return treeset_ctor;
        case StdCollectionsTrap::TREESET_SIZE:
            return treeset_size;
        case StdCollectionsTrap::TREESET_CONTAINS:
            return treeset_contains;
        case StdCollectionsTrap::TREESET_ADD:
            return treeset_add;
        case StdCollectionsTrap::TREESET_REMOVE:
            return treeset_remove;
        case StdCollectionsTrap::TREESET_CLEAR:
            return treeset_clear;
        case StdCollectionsTrap::TREESET_ITERATE:
            return treeset_iterate;
        case StdCollectionsTrap::TREESET_ITERATOR_ALLOC:
            return treeset_iterator_alloc;
        case StdCollectionsTrap::TREESET_ITERATOR_NEXT:
            return treeset_iterator_next;
        case StdCollectionsTrap::TREESET_ITERATOR_VALID:
            return treeset_iterator_valid;
        case StdCollectionsTrap::VECTOR_ALLOC:
            return vector_alloc;
        case StdCollectionsTrap::VECTOR_CTOR:
            return vector_ctor;
        case StdCollectionsTrap::VECTOR_SIZE:
            return vector_size;
        case StdCollectionsTrap::VECTOR_AT:
            return vector_at;
        case StdCollectionsTrap::VECTOR_APPEND:
            return vector_append;
        case StdCollectionsTrap::VECTOR_INSERT:
            return vector_insert;
        case StdCollectionsTrap::VECTOR_UPDATE:
            return vector_update;
        case StdCollectionsTrap::VECTOR_REMOVE:
            return vector_remove;
        case StdCollectionsTrap::VECTOR_CLEAR:
            return vector_clear;
        case StdCollectionsTrap::VECTOR_ITERATE:
            return vector_iterate;
        case StdCollectionsTrap::VECTOR_ITERATOR_ALLOC:
            return vector_iterator_alloc;
        case StdCollectionsTrap::VECTOR_ITERATOR_NEXT:
            return vector_iterator_next;
        case StdCollectionsTrap::VECTOR_ITERATOR_VALID:
            return vector_iterator_valid;

        case StdCollectionsTrap::LAST_:
            break;
    }
    TU_UNREACHABLE();
}

uint32_t
NativeStdCollections::numTraps() const
{
    return static_cast<uint32_t>(StdCollectionsTrap::LAST_);
}

static const NativeStdCollections iface;

const lyric_runtime::NativeInterface *native_init()
{
    return &iface;
}
