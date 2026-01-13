/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_COLLECTIONS_VECTOR_REF_H
#define ZURI_STD_COLLECTIONS_VECTOR_REF_H

#include <absl/container/inlined_vector.h>

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>

class VectorRef : public lyric_runtime::BaseRef {

public:
    explicit VectorRef(const lyric_runtime::VirtualTable *vtable);
    ~VectorRef() override;

    std::string toString() const override;

    lyric_runtime::DataCell at(int index) const;
    lyric_runtime::DataCell first() const;
    lyric_runtime::DataCell last() const;
    int size() const;
    int generation() const;

    void insert(int index, lyric_runtime::DataCell value);
    void append(const lyric_runtime::DataCell &value);
    lyric_runtime::DataCell update(int index, lyric_runtime::DataCell value);
    lyric_runtime::DataCell remove(int index);
    absl::InlinedVector<lyric_runtime::DataCell,16>::iterator begin();
    absl::InlinedVector<lyric_runtime::DataCell,16>::iterator end();
    void clear();

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    absl::InlinedVector<lyric_runtime::DataCell,16> m_seq;
    int m_gen;
};

class VectorIterator : public lyric_runtime::BaseRef {

public:
    explicit VectorIterator(const lyric_runtime::VirtualTable *vtable);
    VectorIterator(
        const lyric_runtime::VirtualTable *vtable,
        VectorRef *vector);

    std::string toString() const override;

    bool iteratorValid() override;
    bool iteratorNext(lyric_runtime::DataCell &next) override;

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    absl::InlinedVector<lyric_runtime::DataCell,16>::iterator m_iter;
    VectorRef *m_vector;
    int m_gen;
};

#endif // ZURI_STD_COLLECTIONS_VECTOR_REF_H