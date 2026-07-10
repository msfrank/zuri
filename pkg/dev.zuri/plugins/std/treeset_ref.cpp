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

tu_uint64
TreeSetRef::getTypeTag() const
{
    return type_tag();
}

void
TreeSetRef::initialize(const TreeSetComparator &cmp)
{
    m_set = absl::btree_set<lyric_runtime::Operand,TreeSetComparator>(cmp);
    m_cmp = cmp;
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
TreeSetRef::contains(const lyric_runtime::Operand &key) const
{
    return m_set.contains(key);
}

lyric_runtime::Operand
TreeSetRef::first() const
{
    auto iterator = m_set.cbegin();
    if (iterator == m_set.cend())
        return {};
    return *iterator;
}

lyric_runtime::Operand
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

lyric_runtime::Operand
TreeSetRef::add(const lyric_runtime::Operand &value)
{
    auto result = m_set.insert(value);
    ++m_gen;
    // return true if value was added, false if value was already present
    return lyric_runtime::Operand::fromBool(result.second);
}

lyric_runtime::Operand
TreeSetRef::remove(const lyric_runtime::Operand &value)
{
    auto result = m_set.extract(value);
    ++m_gen;
    // return true if value was removed, false if value was not present
    return lyric_runtime::Operand::fromBool(!result.empty());
}

lyric_runtime::Operand
TreeSetRef::replace(const lyric_runtime::Operand &value)
{
    // remove the prev value if it exists
    auto result = m_set.extract(value);
    // result is either prev value or undef
    lyric_runtime::Operand prev = result.empty()? lyric_runtime::Operand::undef(): result.value();
    // insert the new value
    m_set.insert(value);
    ++m_gen;
    return prev;
}

absl::btree_set<lyric_runtime::Operand,TreeSetComparator>::iterator
TreeSetRef::begin()
{
    return m_set.begin();
}

absl::btree_set<lyric_runtime::Operand,TreeSetComparator>::iterator
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
        value.setReachable();
    }
}

void
TreeSetRef::clearMembersReachable()
{
    for (auto &value : m_set) {
        value.clearReachable();
    }
}

TreeSetComparator::TreeSetComparator(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::Operand &ctxArgument,
    const lyric_runtime::Operand &compareCall)
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
TreeSetComparator::operator()(const lyric_runtime::Operand& lhs, const lyric_runtime::Operand& rhs) const
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
    tu_int64 cmp;
    TU_ASSERT (ret.getI64(cmp));
    return cmp < 0;
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

tu_uint64
TreeSetIterator::getTypeTag() const
{
    return type_tag();
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
TreeSetIterator::iteratorNext(lyric_runtime::Operand &next)
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
