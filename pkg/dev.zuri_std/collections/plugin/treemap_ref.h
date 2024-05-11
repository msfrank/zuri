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
        const lyric_runtime::DataCell &ord,
        const lyric_runtime::DataCell &cmp);
    TreeMapComparator(const TreeMapComparator &other) noexcept;
    bool operator()(const lyric_runtime::DataCell& lhs, const lyric_runtime::DataCell& rhs) const;

private:
    lyric_runtime::BytecodeInterpreter *m_interp = nullptr;
    lyric_runtime::InterpreterState *m_state = nullptr;
    lyric_runtime::DataCell m_ord;
    lyric_runtime::DataCell m_cmp;
};

class TreeMapRef : public lyric_runtime::BaseRef {

public:
    explicit TreeMapRef(const lyric_runtime::VirtualTable *vtable);
    ~TreeMapRef() override;

    void initialize(const TreeMapComparator &cmp);

    lyric_runtime::DataCell getField(const lyric_runtime::DataCell &field) const override;
    lyric_runtime::DataCell setField(
        const lyric_runtime::DataCell &field,
        const lyric_runtime::DataCell &value) override;
    std::string toString() const override;

    bool contains(const lyric_runtime::DataCell &key) const;
    int size() const;
    lyric_runtime::DataCell get(const lyric_runtime::DataCell &key) const;
    lyric_runtime::DataCell at(int index) const;
    lyric_runtime::DataCell first() const;
    lyric_runtime::DataCell last() const;

    lyric_runtime::DataCell put(const lyric_runtime::DataCell &key, const lyric_runtime::DataCell &value);
    lyric_runtime::DataCell remove(const lyric_runtime::DataCell &key);
    void clear();

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    absl::btree_map<
        lyric_runtime::DataCell,
        lyric_runtime::DataCell,
        TreeMapComparator> m_map;
    TreeMapComparator m_cmp;
};

#endif // ZURI_STD_COLLECTIONS_TREEMAP_REF_H
