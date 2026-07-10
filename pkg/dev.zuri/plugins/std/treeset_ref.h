/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_COLLECTIONS_TREESET_REF_H
#define ZURI_STD_COLLECTIONS_TREESET_REF_H

#include <absl/container/btree_set.h>

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>

class TreeSetComparator {
public:
    TreeSetComparator() = default;
    TreeSetComparator(
        lyric_runtime::BytecodeInterpreter *interp,
        lyric_runtime::InterpreterState *state,
        const lyric_runtime::Operand &ctxArgument,
        const lyric_runtime::Operand &compareCall);
    TreeSetComparator(const TreeSetComparator &other) noexcept;
    bool operator()(const lyric_runtime::Operand& lhs, const lyric_runtime::Operand& rhs) const;

private:
    lyric_runtime::BytecodeInterpreter *m_interp = nullptr;
    lyric_runtime::InterpreterState *m_state = nullptr;
    lyric_runtime::Operand m_ctxArgument;
    lyric_runtime::Operand m_compareCall;
};

class TreeSetRef : public lyric_runtime::BaseRef {

public:
    explicit TreeSetRef(const lyric_runtime::VirtualTable *vtable);
    ~TreeSetRef() override;

    static constexpr tu_uint64 type_tag() { return 0x8a076b4bd49aa78e; }

    tu_uint64 getTypeTag() const override;

    void initialize(const TreeSetComparator &cmp);

    std::string toString() const override;

    bool contains(const lyric_runtime::Operand &value) const;
    int size() const;
    lyric_runtime::Operand first() const;
    lyric_runtime::Operand last() const;
    int generation() const;

    lyric_runtime::Operand add(const lyric_runtime::Operand &value);
    lyric_runtime::Operand remove(const lyric_runtime::Operand &value);
    lyric_runtime::Operand replace(const lyric_runtime::Operand &value);
    absl::btree_set<lyric_runtime::Operand,TreeSetComparator>::iterator begin();
    absl::btree_set<lyric_runtime::Operand,TreeSetComparator>::iterator end();
    void clear();

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    absl::btree_set<lyric_runtime::Operand,TreeSetComparator> m_set;
    int m_gen;
    TreeSetComparator m_cmp;
};

class TreeSetIterator : public lyric_runtime::BaseRef {

public:
    explicit TreeSetIterator(const lyric_runtime::VirtualTable *vtable);
    TreeSetIterator(
        const lyric_runtime::VirtualTable *vtable,
        TreeSetRef *set);

    static constexpr tu_uint64 type_tag() { return 0x25747bd58ae5d4ef; }

    tu_uint64 getTypeTag() const override;

    std::string toString() const override;

    bool iteratorValid() override;
    bool iteratorNext(lyric_runtime::Operand &next) override;

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    absl::btree_set<lyric_runtime::Operand,TreeSetComparator>::iterator m_iter;
    TreeSetRef *m_set;
    int m_gen;
};

#endif // ZURI_STD_COLLECTIONS_TREESET_REF_H
