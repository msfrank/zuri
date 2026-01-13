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
        const lyric_runtime::DataCell &ctxArgument,
        const lyric_runtime::DataCell &compareCall);
    TreeSetComparator(const TreeSetComparator &other) noexcept;
    bool operator()(const lyric_runtime::DataCell& lhs, const lyric_runtime::DataCell& rhs) const;

private:
    lyric_runtime::BytecodeInterpreter *m_interp = nullptr;
    lyric_runtime::InterpreterState *m_state = nullptr;
    lyric_runtime::DataCell m_ctxArgument;
    lyric_runtime::DataCell m_compareCall;
};

class TreeSetRef : public lyric_runtime::BaseRef {

public:
    explicit TreeSetRef(const lyric_runtime::VirtualTable *vtable);
    ~TreeSetRef() override;

    void initialize(const TreeSetComparator &cmp);

    std::string toString() const override;

    bool contains(const lyric_runtime::DataCell &value) const;
    int size() const;
    lyric_runtime::DataCell first() const;
    lyric_runtime::DataCell last() const;
    int generation() const;

    lyric_runtime::DataCell add(const lyric_runtime::DataCell &value);
    lyric_runtime::DataCell remove(const lyric_runtime::DataCell &value);
    lyric_runtime::DataCell replace(const lyric_runtime::DataCell &value);
    absl::btree_set<lyric_runtime::DataCell,TreeSetComparator>::iterator begin();
    absl::btree_set<lyric_runtime::DataCell,TreeSetComparator>::iterator end();
    void clear();

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    absl::btree_set<lyric_runtime::DataCell,TreeSetComparator> m_set;
    int m_gen;
    TreeSetComparator m_cmp;
};

class TreeSetIterator : public lyric_runtime::BaseRef {

public:
    explicit TreeSetIterator(const lyric_runtime::VirtualTable *vtable);
    TreeSetIterator(
        const lyric_runtime::VirtualTable *vtable,
        TreeSetRef *set);

    std::string toString() const override;

    bool iteratorValid() override;
    bool iteratorNext(lyric_runtime::DataCell &next) override;

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    absl::btree_set<lyric_runtime::DataCell,TreeSetComparator>::iterator m_iter;
    TreeSetRef *m_set;
    int m_gen;
};

#endif // ZURI_STD_COLLECTIONS_TREESET_REF_H
