/* SPDX-License-Identifier: BSD-3-Clause */

#include <absl/strings/substitute.h>

#include <tempo_utils/log_stream.h>

#include "vector_ref.h"

VectorRef::VectorRef(const lyric_runtime::VirtualTable *vtable) : BaseRef(vtable)
{
}

VectorRef::~VectorRef()
{
    TU_LOG_INFO << "free" << VectorRef::toString();
    m_seq.clear();
}

lyric_runtime::DataCell
VectorRef::getField(const lyric_runtime::DataCell &field) const
{
    return {};
}

lyric_runtime::DataCell
VectorRef::setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value)
{
    return {};
}

std::string
VectorRef::toString() const
{
    return absl::Substitute("<$0: Vector contains $1 entries>", this, m_seq.size());
}

lyric_runtime::DataCell
VectorRef::at(int index) const
{
    return m_seq[index];
}

lyric_runtime::DataCell
VectorRef::first() const
{
    if (!m_seq.empty())
        return m_seq.front();
    return lyric_runtime::DataCell();
}

lyric_runtime::DataCell
VectorRef::last() const
{
    if (!m_seq.empty())
        return m_seq.back();
    return lyric_runtime::DataCell();
}

absl::InlinedVector<lyric_runtime::DataCell,16>::iterator
VectorRef::begin()
{
    return m_seq.begin();
}

absl::InlinedVector<lyric_runtime::DataCell,16>::iterator
VectorRef::end()
{
    return m_seq.end();
}

int
VectorRef::size() const
{
    return m_seq.size();
}

void
VectorRef::insert(int index, lyric_runtime::DataCell value)
{
    if (0 <= index && index < m_seq.size()) {
        m_seq.insert(m_seq.begin() + index, value);
    } else {
        m_seq.push_back(value);
    }
}

void
VectorRef::append(const lyric_runtime::DataCell &value)
{
    m_seq.push_back(value);
}

lyric_runtime::DataCell
VectorRef::update(int index, lyric_runtime::DataCell value)
{
    if (0 <= index && index < m_seq.size()) {
        auto prev = m_seq[index];
        m_seq[index] = value;
        return prev;
    }
    return lyric_runtime::DataCell();
}

lyric_runtime::DataCell
VectorRef::remove(int index)
{
    if (0 <= index && index < m_seq.size()) {
        auto prev = m_seq[index];
        m_seq.erase(m_seq.begin() + index);
        return prev;
    }
    return lyric_runtime::DataCell();
}

void
VectorRef::clear()
{
    m_seq.clear();
}

void
VectorRef::setMembersReachable()
{
    for (auto &cell : m_seq) {
        if (cell.type == lyric_runtime::DataCellType::REF) {
            TU_ASSERT (cell.data.ref != nullptr);
            cell.data.ref->setReachable();
        }
    }
}

void
VectorRef::clearMembersReachable()
{
    for (auto &cell : m_seq) {
        if (cell.type == lyric_runtime::DataCellType::REF) {
            TU_ASSERT (cell.data.ref != nullptr);
            cell.data.ref->clearReachable();
        }
    }
}

VectorIterator::VectorIterator(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable),
      m_vector(nullptr)
{
}

VectorIterator::VectorIterator(
    const lyric_runtime::VirtualTable *vtable,
    absl::InlinedVector<lyric_runtime::DataCell,16>::iterator iter,
    VectorRef *vector)
    : BaseRef(vtable),
      m_iter(iter),
      m_vector(vector)
{
    TU_ASSERT (m_vector != nullptr);
}

lyric_runtime::DataCell
VectorIterator::getField(const lyric_runtime::DataCell &field) const
{
    return {};
}

lyric_runtime::DataCell
VectorIterator::setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value)
{
    return {};
}

std::string
VectorIterator::toString() const
{
    return absl::Substitute("<$0: VectorIterator>", this);
}

bool
VectorIterator::iteratorValid()
{
    return m_vector && m_iter != m_vector->end();
}

bool
VectorIterator::iteratorNext(lyric_runtime::DataCell &next)
{
    if (!m_vector || m_iter == m_vector->end())
        return false;
    next = *m_iter++;
    return true;
}

void
VectorIterator::setMembersReachable()
{
    m_vector->setReachable();
}

void
VectorIterator::clearMembersReachable()
{
    m_vector->clearReachable();
}
