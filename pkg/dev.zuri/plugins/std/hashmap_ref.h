/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_COLLECTIONS_HASHMAP_REF_H
#define ZURI_STD_COLLECTIONS_HASHMAP_REF_H

#include <functional>
#include <absl/container/flat_hash_map.h>

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>

#include "hashmap_key.h"

class HashMapEq {
public:
    HashMapEq() = default;
    HashMapEq(
        lyric_runtime::BytecodeInterpreter *interp,
        lyric_runtime::InterpreterState *state,
        const lyric_runtime::Operand &ctxArgument,
        const lyric_runtime::Operand &equalsCall);
    HashMapEq(const HashMapEq &other) noexcept;
    bool operator()(const HashMapKey& lhs, const HashMapKey& rhs) const;

private:
    lyric_runtime::BytecodeInterpreter *m_interp = nullptr;
    lyric_runtime::InterpreterState *m_state = nullptr;
    lyric_runtime::Operand m_ctxArgument;
    lyric_runtime::Operand m_equalsCall;
};

using HashMapImpl = absl::flat_hash_map<
    HashMapKey,
    lyric_runtime::Operand,
    absl::flat_hash_map<HashMapKey,lyric_runtime::Operand>::hasher,
    HashMapEq>;

class HashMapRef : public lyric_runtime::BaseRef {

public:
    explicit HashMapRef(const lyric_runtime::VirtualTable *vtable);
    ~HashMapRef() override;

    static constexpr tu_uint64 type_tag() { return 0x75e13d0f6d3c438f; }

    tu_uint64 getTypeTag() const override;

    void initialize(const HashMapEq &eq);

    std::string toString() const override;

    bool contains(const lyric_runtime::Operand &key) const;
    int size() const;
    lyric_runtime::Operand get(const lyric_runtime::Operand &key) const;
    int generation() const;

    lyric_runtime::Operand put(const lyric_runtime::Operand &key, const lyric_runtime::Operand &value);
    lyric_runtime::Operand remove(const lyric_runtime::Operand &key);
    HashMapImpl::iterator begin();
    HashMapImpl::iterator end();
    void clear();

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    HashMapImpl m_map;
    int m_gen;
    HashMapEq m_eq;
};

class HashMapIterator : public lyric_runtime::BaseRef {

public:
    explicit HashMapIterator(const lyric_runtime::VirtualTable *vtable);
    HashMapIterator(
        const lyric_runtime::VirtualTable *vtable,
        HashMapRef *map);

    static constexpr tu_uint64 type_tag() { return 0x4efe207cc6d2f170; }

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
    HashMapImpl::iterator m_iter;
    HashMapRef *m_map;
    int m_gen;
};

#endif // ZURI_STD_COLLECTIONS_HASHMAP_REF_H
