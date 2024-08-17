/* SPDX-License-Identifier: BSD-3-Clause */

#include <absl/strings/substitute.h>

#include <tempo_utils/log_stream.h>

#include "hashmap_ref.h"

HashMapRef::HashMapRef(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable)
{
}

HashMapRef::~HashMapRef()
{
    TU_LOG_INFO << "free" << HashMapRef::toString();
    m_map.clear();
}

void
HashMapRef::initialize(const HashMapEq &eq)
{
    m_map = absl::flat_hash_map<
        HashMapKey,
        lyric_runtime::DataCell,
        absl::flat_hash_map<HashMapKey,lyric_runtime::DataCell>::hasher,
        HashMapEq>(16, absl::flat_hash_map<HashMapKey,lyric_runtime::DataCell>::hasher(), eq);
    m_eq = eq;
}

lyric_runtime::DataCell
HashMapRef::getField(const lyric_runtime::DataCell &field) const
{
    return {};
}

lyric_runtime::DataCell
HashMapRef::setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value)
{
    return {};
}

std::string
HashMapRef::toString() const
{
    return absl::Substitute("<$0: HashMap contains $1 entries>", this, m_map.size());
}

bool
HashMapRef::hashContains(const lyric_runtime::DataCell &key) const
{
    HashMapKey k{ key };
    return m_map.contains(k);
}

int
HashMapRef::hashSize() const
{
    return m_map.size();
}

lyric_runtime::DataCell
HashMapRef::hashGet(const lyric_runtime::DataCell &key) const
{
    HashMapKey k{ key };
    if (!m_map.contains(k))
        return lyric_runtime::DataCell();
    return m_map.at(k);
}

lyric_runtime::DataCell
HashMapRef::hashPut(const lyric_runtime::DataCell &key, const lyric_runtime::DataCell &value)
{
    HashMapKey k{ key };
    auto prev = hashRemove(key);
    m_map[k] = value;
    return prev;
}

lyric_runtime::DataCell
HashMapRef::hashRemove(const lyric_runtime::DataCell &key)
{
    HashMapKey k{ key };
    if (!m_map.contains(k))
        return lyric_runtime::DataCell();
    lyric_runtime::DataCell value = m_map.at(k);
    m_map.erase(k);
    return value;
}

void
HashMapRef::hashClear()
{
    m_map.clear();
}

void
HashMapRef::setMembersReachable()
{
    for (auto iterator = m_map.begin(); iterator != m_map.end(); iterator++) {
        auto &key = iterator->first;
        if (key.cell.type == lyric_runtime::DataCellType::REF) {
            TU_ASSERT (key.cell.data.ref != nullptr);
            key.cell.data.ref->setReachable();
        }
        auto &value = iterator->second;
        if (value.type == lyric_runtime::DataCellType::REF) {
            TU_ASSERT (value.data.ref != nullptr);
            value.data.ref->setReachable();
        }
    }
}

void
HashMapRef::clearMembersReachable()
{
    for (auto iterator = m_map.begin(); iterator != m_map.end(); iterator++) {
        auto &key = iterator->first;
        if (key.cell.type == lyric_runtime::DataCellType::REF) {
            TU_ASSERT (key.cell.data.ref != nullptr);
            key.cell.data.ref->clearReachable();
        }
        auto &value = iterator->second;
        if (value.type == lyric_runtime::DataCellType::REF) {
            TU_ASSERT (value.data.ref != nullptr);
            value.data.ref->clearReachable();
        }
    }
}

HashMapEq::HashMapEq(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::DataCell &eq,
    const lyric_runtime::DataCell &cmp)
    : m_interp(interp), m_state(state), m_eq(eq), m_cmp(cmp)
{
}

HashMapEq::HashMapEq(const HashMapEq &other) noexcept
    : m_interp(other.m_interp), m_state(other.m_state), m_eq(other.m_eq), m_cmp(other.m_cmp)
{
}

bool
HashMapEq::operator()(const HashMapKey& lhs, const HashMapKey& rhs) const
{
    auto *currentCoro = m_state->currentCoro();

    TU_ASSERT (m_cmp.data.descriptor.object == currentCoro->peekSP()->getSegmentIndex());

    std::vector<lyric_runtime::DataCell> args {lhs.cell, rhs.cell, m_eq};
    lyric_runtime::InterpreterStatus status;
    if (!m_state->subroutineManager()->callStatic(m_cmp.data.descriptor.value, args, currentCoro, status))
        return false;

    auto equalsResult = m_interp->runSubinterpreter();
    if (equalsResult.isStatus())
        return false;
    auto ret = equalsResult.getResult();
    return ret.type == lyric_runtime::DataCellType::BOOL && ret.data.b;
}
