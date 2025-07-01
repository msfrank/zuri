/* SPDX-License-Identifier: BSD-3-Clause */

#include <absl/strings/substitute.h>

#include <tempo_utils/log_stream.h>

#include "treemap_ref.h"

TreeMapRef::TreeMapRef(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable),
      m_gen(0)
{
}

TreeMapRef::~TreeMapRef()
{
    TU_LOG_INFO << "free" << TreeMapRef::toString();
    m_map.clear();
}

void
TreeMapRef::initialize(const TreeMapComparator &cmp)
{
    m_map = absl::btree_map<
        lyric_runtime::DataCell,
        lyric_runtime::DataCell,
        TreeMapComparator>(cmp);
    m_cmp = cmp;
}

lyric_runtime::DataCell
TreeMapRef::getField(const lyric_runtime::DataCell &field) const
{
    return {};
}

lyric_runtime::DataCell
TreeMapRef::setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value)
{
    return {};
}

std::string
TreeMapRef::toString() const
{
    return absl::Substitute("<$0: TreeMap contains $1 entries, gen=$2>",
        this, m_map.size(), m_gen);
}

int
TreeMapRef::size() const
{
    return m_map.size();
}

bool
TreeMapRef::contains(const lyric_runtime::DataCell &key) const
{
    return m_map.contains(key);
}

lyric_runtime::DataCell
TreeMapRef::get(const lyric_runtime::DataCell &key) const
{
    if (!m_map.contains(key))
        return {};
    return m_map.at(key);
}

lyric_runtime::DataCell
TreeMapRef::at(int index) const
{
    if (m_map.size() > index) {
        auto iterator = m_map.cbegin();
        std::advance(iterator, index);
        return iterator->second;
    }
    return {};
}

lyric_runtime::DataCell
TreeMapRef::first() const
{
    auto iterator = m_map.cbegin();
    if (iterator == m_map.cend())
        return {};
    return iterator->second;
}

lyric_runtime::DataCell
TreeMapRef::last() const
{
    auto iterator = m_map.crbegin();
    if (iterator == m_map.crend())
        return {};
    return iterator->second;
}

int
TreeMapRef::generation() const
{
    return m_gen;
}

lyric_runtime::DataCell
TreeMapRef::put(const lyric_runtime::DataCell &key, const lyric_runtime::DataCell &value)
{
    auto prev = remove(key);
    m_map[key] = value;
    ++m_gen;
    return prev;
}

lyric_runtime::DataCell
TreeMapRef::remove(const lyric_runtime::DataCell &key)
{
    if (!m_map.contains(key))
        return {};
    lyric_runtime::DataCell value = m_map.at(key);
    m_map.erase(key);
    ++m_gen;
    return value;
}

void
TreeMapRef::clear()
{
    m_map.clear();
    ++m_gen;
}

void
TreeMapRef::setMembersReachable()
{
    for (auto iterator = m_map.begin(); iterator != m_map.end(); iterator++) {
        auto &key = iterator->first;
        if (key.type == lyric_runtime::DataCellType::REF) {
            TU_ASSERT (key.data.ref != nullptr);
            key.data.ref->setReachable();
        }
        auto &value = iterator->second;
        if (value.type == lyric_runtime::DataCellType::REF) {
            TU_ASSERT (value.data.ref != nullptr);
            value.data.ref->setReachable();
        }
    }
}

void
TreeMapRef::clearMembersReachable()
{
    for (auto iterator = m_map.begin(); iterator != m_map.end(); iterator++) {
        auto &key = iterator->first;
        if (key.type == lyric_runtime::DataCellType::REF) {
            TU_ASSERT (key.data.ref != nullptr);
            key.data.ref->clearReachable();
        }
        auto &value = iterator->second;
        if (value.type == lyric_runtime::DataCellType::REF) {
            TU_ASSERT (value.data.ref != nullptr);
            value.data.ref->clearReachable();
        }
    }
}

TreeMapComparator::TreeMapComparator(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::DataCell &ctxArgument,
    const lyric_runtime::DataCell &compareCall)
    : m_interp(interp),
      m_state(state),
      m_ctxArgument(ctxArgument),
      m_compareCall(compareCall)
{
}

TreeMapComparator::TreeMapComparator(const TreeMapComparator &other) noexcept
    : m_interp(other.m_interp),
      m_state(other.m_state),
      m_ctxArgument(other.m_ctxArgument),
      m_compareCall(other.m_compareCall)
{
}

bool
TreeMapComparator::operator()(const lyric_runtime::DataCell& lhs, const lyric_runtime::DataCell& rhs) const
{
    auto *currentCoro = m_state->currentCoro();

    std::vector args {lhs, rhs, m_ctxArgument};
    tempo_utils::Status status;

    if (!m_state->subroutineManager()->callStatic(m_compareCall, args, currentCoro, status))
        return false;

    auto compareResult = m_interp->runSubinterpreter();
    if (compareResult.isStatus())
        return false;
    auto ret = compareResult.getResult();
    if (ret.type == lyric_runtime::DataCellType::I64)
        return ret.data.i64 < 0;
    return false;
}