/* SPDX-License-Identifier: BSD-3-Clause */

#include <absl/strings/substitute.h>

#include <lyric_object/bytecode_iterator.h>
#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/serialize_value.h>
#include <lyric_runtime/url_ref.h>
#include <lyric_serde/patchset_value.h>
#include <tempo_utils/big_endian.h>
#include <tempo_utils/log_stream.h>

#include "attr_ref.h"

AttrRef::AttrRef(
    const lyric_runtime::VirtualTable *vtable,
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state)
    : BaseValueRef(vtable),
      m_interp(interp),
      m_state(state)
{
    TU_ASSERT (m_interp != nullptr);
    TU_ASSERT (m_state != nullptr);
    m_fields.resize(vtable->getLayoutTotal());
}

AttrRef::~AttrRef()
{
    TU_LOG_INFO << "free" << AttrRef::toString();
}

ValueType
AttrRef::getValueType() const
{
    return ValueType::Attr;
}

lyric_runtime::DataCell
AttrRef::getField(const lyric_runtime::DataCell &field) const
{
    auto *vtable = getVirtualTable();
    auto *member = vtable->getMember(field);
    if (member == nullptr)
        return {};
    auto offset = member->getLayoutOffset();
    if (m_fields.size() <= offset)
        return {};
    return m_fields.at(offset);
}

lyric_runtime::DataCell
AttrRef::setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value)
{
    auto *vtable = getVirtualTable();
    auto *member = vtable->getMember(field);
    if (member == nullptr)
        return {};
    auto offset = member->getLayoutOffset();
    if (m_fields.size() <= offset)
        return {};
    auto prev = m_fields.at(offset);
    m_fields[offset] = value;
    return prev;
}

bool
AttrRef::serializeValue(lyric_serde::PatchsetState &state, tu_uint32 &index)
{
    auto *currentCoro = m_state->currentCoro();
    auto *segment = currentCoro->peekSP();
    auto object = segment->getObject().getObject();

    auto *segmentManager = m_state->segmentManager();

    lyric_runtime::InterpreterStatus status;

    // get ns field
    auto symbol = object.findSymbol(lyric_common::SymbolPath({"Attr", "ns"}));
    TU_ASSERT (symbol.isValid());
    auto descriptor = segmentManager->resolveDescriptor(segment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    TU_ASSERT (descriptor.type == lyric_runtime::DataCellType::FIELD);
    auto ns = AttrRef::getField(descriptor);
    if (ns.type != lyric_runtime::DataCellType::URL)
        return false;
    tempo_utils::Url nsUrl;
    if (!ns.data.url->uriValue(nsUrl))
        return false;
    auto nsString = nsUrl.toString();

    // get id field
    symbol = object.findSymbol(lyric_common::SymbolPath({"Attr", "id"}));
    TU_ASSERT (symbol.isValid());
    descriptor = segmentManager->resolveDescriptor(segment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    TU_ASSERT (descriptor.type == lyric_runtime::DataCellType::FIELD);
    auto id = AttrRef::getField(descriptor);
    if (id.type != lyric_runtime::DataCellType::I64)
        return false;
    if (std::numeric_limits<tu_uint32>::max() < id.data.i64)
        return false;
    auto idInt = static_cast<tu_uint32>(id.data.i64);

    // get value field
    symbol = object.findSymbol(lyric_common::SymbolPath({"Attr", "value"}));
    TU_ASSERT (symbol.isValid());
    descriptor = segmentManager->resolveDescriptor(segment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    TU_ASSERT (descriptor.type == lyric_runtime::DataCellType::FIELD);
    auto value = AttrRef::getField(descriptor);

    // serialize the value in the writer
    auto valueOffset = lyric_runtime::serialize_value(value, state);
    if (valueOffset == lyric_runtime::INVALID_ADDRESS_U32)
        return false;

    auto appendValueResult = state.appendValue(nsString.c_str(), static_cast<tu_uint32>(idInt),
        lyric_serde::ValueAddress(valueOffset));
    if (appendValueResult.isStatus())
        return false;

    index = appendValueResult.getResult()->getAddress().getAddress();
    return true;
}

std::string
AttrRef::toString() const
{
    return absl::Substitute("<$0: AttrRef>", this);
}

void
AttrRef::setMembersReachable()
{
    for (auto &cell : m_fields) {
        if (cell.type == lyric_runtime::DataCellType::REF) {
            TU_ASSERT (cell.data.ref != nullptr);
            cell.data.ref->setReachable();
        }
    }
}

void
AttrRef::clearMembersReachable()
{
    for (auto &cell : m_fields) {
        if (cell.type == lyric_runtime::DataCellType::REF) {
            TU_ASSERT (cell.data.ref != nullptr);
            cell.data.ref->clearReachable();
        }
    }
}

tempo_utils::Status
attr_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();
    const auto *vtable = frame.getVirtualTable();
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<AttrRef>(vtable, interp, state);
    currentCoro->pushData(ref);

    return lyric_runtime::InterpreterStatus::ok();
}