/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_COLLECTIONS_HASHMAP_REF_H
#define ZURI_STD_COLLECTIONS_HASHMAP_REF_H

#include <functional>
#include <absl/container/flat_hash_map.h>

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>

#include "hashmap_key.h"

struct HashMapEq {
public:
    HashMapEq() = default;
    HashMapEq(
        lyric_runtime::BytecodeInterpreter *interp,
        lyric_runtime::InterpreterState *state,
        const lyric_runtime::DataCell &ctxArgument,
        const lyric_runtime::DataCell &equalsCall);
    HashMapEq(const HashMapEq &other) noexcept;
    bool operator()(const HashMapKey& lhs, const HashMapKey& rhs) const;

private:
    lyric_runtime::BytecodeInterpreter *m_interp = nullptr;
    lyric_runtime::InterpreterState *m_state = nullptr;
    lyric_runtime::DataCell m_ctxArgument;
    lyric_runtime::DataCell m_equalsCall;
};

class HashMapRef : public lyric_runtime::BaseRef {

public:
    explicit HashMapRef(const lyric_runtime::VirtualTable *vtable);
    ~HashMapRef() override;

    void initialize(const HashMapEq &eq);

    lyric_runtime::DataCell getField(const lyric_runtime::DataCell &field) const override;
    lyric_runtime::DataCell setField(
        const lyric_runtime::DataCell &field,
        const lyric_runtime::DataCell &value) override;
    std::string toString() const override;

    bool contains(const lyric_runtime::DataCell &key) const;
    int size() const;
    lyric_runtime::DataCell get(const lyric_runtime::DataCell &key) const;
    int generation() const;

    lyric_runtime::DataCell put(const lyric_runtime::DataCell &key, const lyric_runtime::DataCell &value);
    lyric_runtime::DataCell remove(const lyric_runtime::DataCell &key);
    void clear();

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    absl::flat_hash_map<
        HashMapKey,
        lyric_runtime::DataCell,
        absl::flat_hash_map<HashMapKey,lyric_runtime::DataCell>::hasher,
        HashMapEq> m_map;
    int m_gen;
    HashMapEq m_eq;
};

#endif // ZURI_STD_COLLECTIONS_HASHMAP_REF_H
