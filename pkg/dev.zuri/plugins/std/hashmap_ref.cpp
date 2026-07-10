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

tu_uint64
HashMapRef::getTypeTag() const
{
    return type_tag();
}

void
HashMapRef::initialize(const HashMapEq &eq)
{
    m_map = absl::flat_hash_map<
        HashMapKey,
        lyric_runtime::Operand,
        absl::flat_hash_map<HashMapKey,lyric_runtime::Operand>::hasher,
        HashMapEq>(16, absl::flat_hash_map<HashMapKey,lyric_runtime::Operand>::hasher(), eq);
    m_eq = eq;
}

std::string
HashMapRef::toString() const
{
    return absl::Substitute("<$0: HashMap contains $1 entries, gen=$2>",
        this, m_map.size(), m_gen);
}

bool
HashMapRef::contains(const lyric_runtime::Operand &key) const
{
    HashMapKey k{ key };
    return m_map.contains(k);
}

int
HashMapRef::size() const
{
    return m_map.size();
}

lyric_runtime::Operand
HashMapRef::get(const lyric_runtime::Operand &key) const
{
    HashMapKey k{ key };
    if (!m_map.contains(k))
        return {};
    return m_map.at(k);
}

int
HashMapRef::generation() const
{
    return m_gen;
}

lyric_runtime::Operand
HashMapRef::put(const lyric_runtime::Operand &key, const lyric_runtime::Operand &value)
{
    HashMapKey k{ key };
    auto prev = remove(key);
    m_map[k] = value;
    ++m_gen;
    return prev;
}

lyric_runtime::Operand
HashMapRef::remove(const lyric_runtime::Operand &key)
{
    HashMapKey k{ key };
    if (!m_map.contains(k))
        return {};
    lyric_runtime::Operand value = m_map.at(k);
    m_map.erase(k);
    ++m_gen;
    return value;
}

HashMapImpl::iterator
HashMapRef::begin()
{
    return m_map.begin();
}

HashMapImpl::iterator
HashMapRef::end()
{
    return m_map.end();
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
        key.cell.setReachable();
        auto &value = iterator->second;
        value.setReachable();
    }
}

void
HashMapRef::clearMembersReachable()
{
    for (auto iterator = m_map.begin(); iterator != m_map.end(); iterator++) {
        auto &key = iterator->first;
        key.cell.clearReachable();
        auto &value = iterator->second;
        value.clearReachable();
    }
}

HashMapEq::HashMapEq(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::Operand &ctxArgument,
    const lyric_runtime::Operand &equalsCall)
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
    bool eq;
    TU_ASSERT (ret.getBool(eq));
    return eq;
}

HashMapIterator::HashMapIterator(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable),
      m_map(nullptr),
      m_gen(0)
{
}

HashMapIterator::HashMapIterator(
    const lyric_runtime::VirtualTable *vtable,
    HashMapRef *map)
    : BaseRef(vtable),
      m_map(map)
{
    TU_ASSERT (m_map != nullptr);
    m_iter = map->begin();
    m_gen = map->generation();
}

tu_uint64
HashMapIterator::getTypeTag() const
{
    return type_tag();
}

std::string
HashMapIterator::toString() const
{
    return absl::Substitute("<$0: HashMapIterator gen=$1>", this, m_gen);
}

bool
HashMapIterator::valid()
{
    return m_map && m_iter != m_map->end() && m_gen == m_map->generation();
}

lyric_runtime::Operand
HashMapIterator::key()
{
    if (!valid())
        return {};
    return m_iter->first.cell;
}

lyric_runtime::Operand
HashMapIterator::value()
{
    if (!valid())
        return {};
    return m_iter->second;
}

bool
HashMapIterator::next()
{
    if (!valid())
        return false;
    ++m_iter;
    return true;
}

void
HashMapIterator::setMembersReachable()
{
    m_map->setReachable();
}

void
HashMapIterator::clearMembersReachable()
{
    m_map->clearReachable();
}
