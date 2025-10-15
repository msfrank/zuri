/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_COLLECTIONS_TREEMAP_REF_H
#define ZURI_STD_COLLECTIONS_TREEMAP_REF_H

#include <absl/container/btree_map.h>

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>

class TreeMapComparator {
public:
    TreeMapComparator() = default;
    TreeMapComparator(
        lyric_runtime::BytecodeInterpreter *interp,
        lyric_runtime::InterpreterState *state,
        const lyric_runtime::DataCell &ctxArgument,
        const lyric_runtime::DataCell &compareCall);
    TreeMapComparator(const TreeMapComparator &other) noexcept;
    bool operator()(const lyric_runtime::DataCell& lhs, const lyric_runtime::DataCell& rhs) const;

private:
    lyric_runtime::BytecodeInterpreter *m_interp = nullptr;
    lyric_runtime::InterpreterState *m_state = nullptr;
    lyric_runtime::DataCell m_ctxArgument;
    lyric_runtime::DataCell m_compareCall;
};

using TreeMapImpl = absl::btree_map<
    lyric_runtime::DataCell,
    lyric_runtime::DataCell,
    TreeMapComparator>;

class TreeMapRef : public lyric_runtime::BaseRef {

public:
    explicit TreeMapRef(const lyric_runtime::VirtualTable *vtable);
    ~TreeMapRef() override;

    void initialize(const TreeMapComparator &cmp);

    std::string toString() const override;

    bool contains(const lyric_runtime::DataCell &key) const;
    int size() const;
    lyric_runtime::DataCell get(const lyric_runtime::DataCell &key) const;
    lyric_runtime::DataCell at(int index) const;
    lyric_runtime::DataCell first() const;
    lyric_runtime::DataCell last() const;
    int generation() const;

    lyric_runtime::DataCell put(const lyric_runtime::DataCell &key, const lyric_runtime::DataCell &value);
    lyric_runtime::DataCell remove(const lyric_runtime::DataCell &key);
    TreeMapImpl::iterator begin();
    TreeMapImpl::iterator end();
    void clear();

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    TreeMapImpl m_map;
    int m_gen;
    TreeMapComparator m_cmp;
};

class TreeMapIterator : public lyric_runtime::BaseRef {

public:
    explicit TreeMapIterator(const lyric_runtime::VirtualTable *vtable);
    TreeMapIterator(
        const lyric_runtime::VirtualTable *vtable,
        TreeMapRef *map);

    std::string toString() const override;

    bool valid();
    lyric_runtime::DataCell key();
    lyric_runtime::DataCell value();
    bool next();

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    TreeMapImpl::iterator m_iter;
    TreeMapRef *m_map;
    int m_gen;
};

#endif // ZURI_STD_COLLECTIONS_TREEMAP_REF_H
