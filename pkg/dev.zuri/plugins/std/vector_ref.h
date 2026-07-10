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

    static constexpr tu_uint64 type_tag() { return 0x5fb34c751db65bb2; }

    tu_uint64 getTypeTag() const override;

    std::string toString() const override;

    lyric_runtime::Operand at(int index) const;
    lyric_runtime::Operand first() const;
    lyric_runtime::Operand last() const;
    int size() const;
    int generation() const;

    void insert(int index, lyric_runtime::Operand value);
    void append(const lyric_runtime::Operand &value);
    lyric_runtime::Operand update(int index, lyric_runtime::Operand value);
    lyric_runtime::Operand remove(int index);
    absl::InlinedVector<lyric_runtime::Operand,16>::iterator begin();
    absl::InlinedVector<lyric_runtime::Operand,16>::iterator end();
    void clear();

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    absl::InlinedVector<lyric_runtime::Operand,16> m_seq;
    int m_gen;
};

class VectorIterator : public lyric_runtime::BaseRef {

public:
    explicit VectorIterator(const lyric_runtime::VirtualTable *vtable);
    VectorIterator(
        const lyric_runtime::VirtualTable *vtable,
        VectorRef *vector);

    static constexpr tu_uint64 type_tag() { return 0xa093009f388c2b44; }

    tu_uint64 getTypeTag() const override;

    std::string toString() const override;

    bool iteratorValid() override;
    bool iteratorNext(lyric_runtime::Operand &next) override;

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    absl::InlinedVector<lyric_runtime::Operand,16>::iterator m_iter;
    VectorRef *m_vector;
    int m_gen;
};

#endif // ZURI_STD_COLLECTIONS_VECTOR_REF_H