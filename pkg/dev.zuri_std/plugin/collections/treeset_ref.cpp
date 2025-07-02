/* SPDX-License-Identifier: BSD-3-Clause */

#include <absl/strings/substitute.h>

#include <tempo_utils/log_stream.h>

#include "treeset_ref.h"

TreeSetRef::TreeSetRef(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable),
      m_gen(0)
{
}

TreeSetRef::~TreeSetRef()
{
    TU_LOG_INFO << "free" << TreeSetRef::toString();
    m_set.clear();
}

void
TreeSetRef::initialize(const TreeSetComparator &cmp)
{
    m_set = absl::btree_set<lyric_runtime::DataCell,TreeSetComparator>(cmp);
    m_cmp = cmp;
}

lyric_runtime::DataCell
TreeSetRef::getField(const lyric_runtime::DataCell &field) const
{
    return {};
}

lyric_runtime::DataCell
TreeSetRef::setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value)
{
    return {};
}

std::string
TreeSetRef::toString() const
{
    return absl::Substitute("<$0: TreeSet with $1 entries, gen=$2>",
        this, m_set.size(), m_gen);
}

int
TreeSetRef::size() const
{
    return m_set.size();
}

bool
TreeSetRef::contains(const lyric_runtime::DataCell &key) const
{
    return m_set.contains(key);
}

lyric_runtime::DataCell
TreeSetRef::first() const
{
    auto iterator = m_set.cbegin();
    if (iterator == m_set.cend())
        return {};
    return *iterator;
}

lyric_runtime::DataCell
TreeSetRef::last() const
{
    auto iterator = m_set.crbegin();
    if (iterator == m_set.crend())
        return {};
    return *iterator;
}

int
TreeSetRef::generation() const
{
    return m_gen;
}

lyric_runtime::DataCell
TreeSetRef::add(const lyric_runtime::DataCell &value)
{
    auto result = m_set.insert(value);
    ++m_gen;
    // return true if value was added, false if value was already present
    return lyric_runtime::DataCell(result.second);
}

lyric_runtime::DataCell
TreeSetRef::remove(const lyric_runtime::DataCell &value)
{
    auto result = m_set.extract(value);
    ++m_gen;
    // return true if value was removed, false if value was not present
    return lyric_runtime::DataCell(!result.empty());
}

lyric_runtime::DataCell
TreeSetRef::replace(const lyric_runtime::DataCell &value)
{
    // remove the prev value if it exists
    auto result = m_set.extract(value);
    // result is either prev value or nil
    lyric_runtime::DataCell prev = result.empty()? lyric_runtime::DataCell::nil(): result.value();
    // insert the new value
    m_set.insert(value);
    ++m_gen;
    return prev;
}

absl::btree_set<lyric_runtime::DataCell,TreeSetComparator>::iterator
TreeSetRef::begin()
{
    return m_set.begin();
}

absl::btree_set<lyric_runtime::DataCell,TreeSetComparator>::iterator
TreeSetRef::end()
{
    return m_set.end();
}

void
TreeSetRef::clear()
{
    m_set.clear();
    ++m_gen;
}

void
TreeSetRef::setMembersReachable()
{
    for (auto &value : m_set) {
        if (value.type == lyric_runtime::DataCellType::REF) {
            TU_ASSERT (value.data.ref != nullptr);
            value.data.ref->setReachable();
        }
    }
}

void
TreeSetRef::clearMembersReachable()
{
    for (auto &value : m_set) {
        if (value.type == lyric_runtime::DataCellType::REF) {
            TU_ASSERT (value.data.ref != nullptr);
            value.data.ref->clearReachable();
        }
    }
}

TreeSetComparator::TreeSetComparator(
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

TreeSetComparator::TreeSetComparator(const TreeSetComparator &other) noexcept
    : m_interp(other.m_interp),
      m_state(other.m_state),
      m_ctxArgument(other.m_ctxArgument),
      m_compareCall(other.m_compareCall)
{
}

bool
TreeSetComparator::operator()(const lyric_runtime::DataCell& lhs, const lyric_runtime::DataCell& rhs) const
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

TreeSetIterator::TreeSetIterator(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable),
      m_set(nullptr),
      m_gen(0)
{
}

TreeSetIterator::TreeSetIterator(
    const lyric_runtime::VirtualTable *vtable,
    TreeSetRef *set)
    : BaseRef(vtable),
      m_set(set)
{
    TU_ASSERT (m_set != nullptr);
    m_iter = set->begin();
    m_gen = set->generation();
}

lyric_runtime::DataCell
TreeSetIterator::getField(const lyric_runtime::DataCell &field) const
{
    return {};
}

lyric_runtime::DataCell
TreeSetIterator::setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value)
{
    return {};
}

std::string
TreeSetIterator::toString() const
{
    return absl::Substitute("<$0: TreeSetIterator gen=$1>", this, m_gen);
}

bool
TreeSetIterator::iteratorValid()
{
    return m_set && m_iter != m_set->end() && m_gen == m_set->generation();
}

bool
TreeSetIterator::iteratorNext(lyric_runtime::DataCell &next)
{
    if (!iteratorValid())
        return false;
    next = *m_iter++;
    return true;
}

void
TreeSetIterator::setMembersReachable()
{
    m_set->setReachable();
}

void
TreeSetIterator::clearMembersReachable()
{
    m_set->clearReachable();
}
