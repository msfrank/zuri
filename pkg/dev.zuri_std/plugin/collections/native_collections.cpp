/* SPDX-License-Identifier: BSD-3-Clause */

#include <vector>

#include <tempo_utils/log_stream.h>

#include "hashmap_traps.h"
#include "native_collections.h"
#include "treemap_traps.h"
#include "treeset_traps.h"
#include "vector_traps.h"

std::array<lyric_runtime::NativeTrap,40> kStdCollectionsTraps = {{
    {hashmap_alloc, "STD_COLLECTIONS_HASHMAP_ALLOC", 0},
    {hashmap_ctor, "STD_COLLECTIONS_HASHMAP_CTOR", 0},
    {hashmap_size, "STD_COLLECTIONS_HASHMAP_SIZE", 0},
    {hashmap_contains, "STD_COLLECTIONS_HASHMAP_CONTAINS", 0},
    {hashmap_get, "STD_COLLECTIONS_HASHMAP_GET", 0},
    {hashmap_put, "STD_COLLECTIONS_HASHMAP_PUT", 0},
    {hashmap_remove, "STD_COLLECTIONS_HASHMAP_REMOVE", 0},
    {hashmap_clear, "STD_COLLECTIONS_HASHMAP_CLEAR", 0},
    {treemap_alloc, "STD_COLLECTIONS_TREEMAP_ALLOC", 0},
    {treemap_ctor, "STD_COLLECTIONS_TREEMAP_CTOR", 0},
    {treemap_size, "STD_COLLECTIONS_TREEMAP_SIZE", 0},
    {treemap_contains, "STD_COLLECTIONS_TREEMAP_CONTAINS", 0},
    {treemap_get, "STD_COLLECTIONS_TREEMAP_GET", 0},
    {treemap_put, "STD_COLLECTIONS_TREEMAP_PUT", 0},
    {treemap_remove, "STD_COLLECTIONS_TREEMAP_REMOVE", 0},
    {treemap_clear, "STD_COLLECTIONS_TREEMAP_CLEAR", 0},
    {treeset_alloc, "STD_COLLECTIONS_TREESET_ALLOC", 0},
    {treeset_ctor, "STD_COLLECTIONS_TREESET_CTOR", 0},
    {treeset_size, "STD_COLLECTIONS_TREESET_SIZE", 0},
    {treeset_contains, "STD_COLLECTIONS_TREESET_CONTAINS", 0},
    {treeset_add, "STD_COLLECTIONS_TREESET_ADD", 0},
    {treeset_remove, "STD_COLLECTIONS_TREESET_REMOVE", 0},
    {treeset_clear, "STD_COLLECTIONS_TREESET_CLEAR", 0},
    {treeset_iterate, "STD_COLLECTIONS_TREESET_ITERATE", 0},
    {treeset_iterator_alloc, "STD_COLLECTIONS_TREESET_ITERATOR_ALLOC", 0},
    {treeset_iterator_next, "STD_COLLECTIONS_TREESET_ITERATOR_NEXT", 0},
    {treeset_iterator_valid, "STD_COLLECTIONS_TREESET_ITERATOR_VALID", 0},
    {vector_alloc, "STD_COLLECTIONS_VECTOR_ALLOC", 0},
    {vector_ctor, "STD_COLLECTIONS_VECTOR_CTOR", 0},
    {vector_size, "STD_COLLECTIONS_VECTOR_SIZE", 0},
    {vector_at, "STD_COLLECTIONS_VECTOR_AT", 0},
    {vector_append, "STD_COLLECTIONS_VECTOR_APPEND", 0},
    {vector_insert, "STD_COLLECTIONS_VECTOR_INSERT", 0},
    {vector_update, "STD_COLLECTIONS_VECTOR_UPDATE", 0},
    {vector_remove, "STD_COLLECTIONS_VECTOR_REMOVE", 0},
    {vector_clear, "STD_COLLECTIONS_VECTOR_CLEAR", 0},
    {vector_iterate, "STD_COLLECTIONS_VECTOR_ITERATE", 0},
    {vector_iterator_alloc, "STD_COLLECTIONS_VECTOR_ITERATOR_ALLOC", 0},
    {vector_iterator_next, "STD_COLLECTIONS_VECTOR_ITERATOR_NEXT", 0},
    {vector_iterator_valid, "STD_COLLECTIONS_VECTOR_ITERATOR_VALID", 0}
}};

class NativeStdCollections : public lyric_runtime::NativeInterface {

public:
    NativeStdCollections() = default;
    bool load(lyric_runtime::BytecodeSegment *segment) const override;
    void unload(lyric_runtime::BytecodeSegment *segment) const override;
    const lyric_runtime::NativeTrap *getTrap(uint32_t index) const override;
    uint32_t numTraps() const override;
};

const lyric_runtime::NativeTrap *
NativeStdCollections::getTrap(uint32_t index) const
{
    if (kStdCollectionsTraps.size() <= index)
        return nullptr;
    return &kStdCollectionsTraps.at(index);
}

bool
NativeStdCollections::load(lyric_runtime::BytecodeSegment *segment) const
{
    return true;
}

void
NativeStdCollections::unload(lyric_runtime::BytecodeSegment *segment) const
{
}

uint32_t
NativeStdCollections::numTraps() const
{
    return kStdCollectionsTraps.size();
}

static const NativeStdCollections iface;

const lyric_runtime::NativeInterface *native_init()
{
    return &iface;
}
