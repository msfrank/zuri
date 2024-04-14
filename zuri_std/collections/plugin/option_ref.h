/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef ZURI_STD_COLLECTIONS_OPTION_REF_H
#define ZURI_STD_COLLECTIONS_OPTION_REF_H

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>

class OptionRef : public lyric_runtime::BaseRef {

public:
    explicit OptionRef(const lyric_runtime::VirtualTable *vtable);
    ~OptionRef() override;

    lyric_runtime::DataCell getField(const lyric_runtime::DataCell &field) const override;
    lyric_runtime::DataCell setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value) override;
    bool hashValue(absl::HashState state) override;
    std::string toString() const override;

    lyric_runtime::DataCell optionGet() const;
    bool optionSet(const lyric_runtime::DataCell &value);

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    lyric_runtime::DataCell m_value;
};

//class OptionIterator : public lyric_runtime::BaseRef {
//
//public:
//    explicit OptionIterator(const lyric_runtime::VirtualTable *vtable);
//    OptionIterator(const lyric_runtime::VirtualTable *vtable, const lyric_runtime::DataCell &value);
//
//    lyric_runtime::DataCell getField(const lyric_runtime::DataCell &field) const override;
//    lyric_runtime::DataCell setField(
//        const lyric_runtime::DataCell &field,
//        const lyric_runtime::DataCell &value) override;
//    std::string toString() const override;
//
//    bool iteratorValid() override;
//    bool iteratorNext(lyric_runtime::DataCell &next) override;
//
//protected:
//    void setMembersReachable() override;
//    void clearMembersReachable() override;
//
//private:
//    lyric_runtime::DataCell m_value;
//};

tempo_utils::Status option_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status option_ctor(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status option_get(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status option_is_empty(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);

#endif // ZURI_STD_COLLECTIONS_OPTION_REF_H