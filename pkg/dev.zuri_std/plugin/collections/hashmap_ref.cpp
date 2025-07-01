/* SPDX-License-Identifier: BSD-3-Clause */

#include <absl/strings/substitute.h>

#include <tempo_utils/log_stream.h>

#include "hashmap_ref.h"

HashMapRef::HashMapRef(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable),
      m_gen(0)
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
    return absl::Substitute("<$0: HashMap contains $1 entries, gen=$2>",
        this, m_map.size(), m_gen);
}

bool
HashMapRef::contains(const lyric_runtime::DataCell &key) const
{
    HashMapKey k{ key };
    return m_map.contains(k);
}

int
HashMapRef::size() const
{
    return m_map.size();
}

lyric_runtime::DataCell
HashMapRef::get(const lyric_runtime::DataCell &key) const
{
    HashMapKey k{ key };
    if (!m_map.contains(k))
        return {};
    return m_map.at(k);
}

lyric_runtime::DataCell
HashMapRef::put(const lyric_runtime::DataCell &key, const lyric_runtime::DataCell &value)
{
    HashMapKey k{ key };
    auto prev = remove(key);
    m_map[k] = value;
    ++m_gen;
    return prev;
}

lyric_runtime::DataCell
HashMapRef::remove(const lyric_runtime::DataCell &key)
{
    HashMapKey k{ key };
    if (!m_map.contains(k))
        return {};
    lyric_runtime::DataCell value = m_map.at(k);
    m_map.erase(k);
    ++m_gen;
    return value;
}

void
HashMapRef::clear()
{
    m_map.clear();
    ++m_gen;
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
    const lyric_runtime::DataCell &ctxArgument,
    const lyric_runtime::DataCell &equalsCall)
    : m_interp(interp),
      m_state(state),
      m_ctxArgument(ctxArgument),
      m_equalsCall(equalsCall)
{
}

HashMapEq::HashMapEq(const HashMapEq &other) noexcept
    : m_interp(other.m_interp),
      m_state(other.m_state),
      m_ctxArgument(other.m_ctxArgument),
      m_equalsCall(other.m_equalsCall)
{
}

bool
HashMapEq::operator()(const HashMapKey& lhs, const HashMapKey& rhs) const
{
    auto *currentCoro = m_state->currentCoro();
    auto *subroutineManager = m_state->subroutineManager();

    std::vector args {lhs.cell, rhs.cell, m_ctxArgument};
    lyric_runtime::InterpreterStatus status;

    if (!subroutineManager->callStatic(m_equalsCall, args, currentCoro, status))
        return false;

    auto equalsResult = m_interp->runSubinterpreter();
    if (equalsResult.isStatus())
        return false;
    auto ret = equalsResult.getResult();
    return ret.type == lyric_runtime::DataCellType::BOOL && ret.data.b;
}
