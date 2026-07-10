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
        const lyric_runtime::Operand &ctxArgument,
        const lyric_runtime::Operand &compareCall);
    TreeMapComparator(const TreeMapComparator &other) noexcept;
    bool operator()(const lyric_runtime::Operand& lhs, const lyric_runtime::Operand& rhs) const;

private:
    lyric_runtime::BytecodeInterpreter *m_interp = nullptr;
    lyric_runtime::InterpreterState *m_state = nullptr;
    lyric_runtime::Operand m_ctxArgument;
    lyric_runtime::Operand m_compareCall;
};

using TreeMapImpl = absl::btree_map<
    lyric_runtime::Operand,
    lyric_runtime::Operand,
    TreeMapComparator>;

class TreeMapRef : public lyric_runtime::BaseRef {

public:
    explicit TreeMapRef(const lyric_runtime::VirtualTable *vtable);
    ~TreeMapRef() override;

    static constexpr tu_uint64 type_tag() { return 0x69bcc183470b9c85; }

    tu_uint64 getTypeTag() const override;

    void initialize(const TreeMapComparator &cmp);

    std::string toString() const override;

    bool contains(const lyric_runtime::Operand &key) const;
    int size() const;
    lyric_runtime::Operand get(const lyric_runtime::Operand &key) const;
    lyric_runtime::Operand at(int index) const;
    lyric_runtime::Operand first() const;
    lyric_runtime::Operand last() const;
    int generation() const;

    lyric_runtime::Operand put(const lyric_runtime::Operand &key, const lyric_runtime::Operand &value);
    lyric_runtime::Operand remove(const lyric_runtime::Operand &key);
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

    static constexpr tu_uint64 type_tag() { return 0xa2727353811b3cb; }

    tu_uint64 getTypeTag() const override;

    std::string toString() const override;

    bool valid();
    lyric_runtime::Operand key();
    lyric_runtime::Operand value();
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
