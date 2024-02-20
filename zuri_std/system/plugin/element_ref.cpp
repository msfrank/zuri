/* SPDX-License-Identifier: BSD-3-Clause */

#include <absl/strings/substitute.h>

#include <lyric_object/bytecode_iterator.h>
#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/serialize_value.h>
#include <lyric_serde/patchset_value.h>
#include <tempo_utils/big_endian.h>
#include <tempo_utils/log_stream.h>

#include "element_ref.h"

ElementRef::ElementRef(
    const lyric_runtime::VirtualTable *vtable,
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state)
    : BaseRef(vtable),
      m_interp(interp),
      m_state(state)
{
    TU_ASSERT (m_interp != nullptr);
    TU_ASSERT (m_state != nullptr);
    m_fields.resize(vtable->getLayoutTotal());
}

ElementRef::~ElementRef()
{
    TU_LOG_INFO << "free" << ElementRef::toString();
}

lyric_runtime::DataCell
ElementRef::getField(const lyric_runtime::DataCell &field) const
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
ElementRef::setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value)
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
ElementRef::serializeValue(lyric_serde::PatchsetState &state, tu_uint32 &index)
{
    auto *currentCoro = m_state->currentCoro();
    auto *segment = currentCoro->peekSP();
    auto object = segment->getObject().getObject();

    auto *segmentManager = m_state->segmentManager();
    auto *subroutineManager = m_state->subroutineManager();

    lyric_runtime::InterpreterStatus status;

    // get ns field
    auto symbol = object.findSymbol(lyric_common::SymbolPath({"Element", "ns"}));
    TU_ASSERT (symbol.isValid());
    auto descriptor = segmentManager->resolveDescriptor(segment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    TU_ASSERT (descriptor.type == lyric_runtime::DataCellType::FIELD);
    auto ns = ElementRef::getField(descriptor);
    if (ns.type != lyric_runtime::DataCellType::REF)
        return false;
    tempo_utils::Url nsUrl;
    if (!ns.data.ref->uriValue(nsUrl))
        return false;
    auto nsString = nsUrl.toString();

    // get id field
    symbol = object.findSymbol(lyric_common::SymbolPath({"Element", "id"}));
    TU_ASSERT (symbol.isValid());
    descriptor = segmentManager->resolveDescriptor(segment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    TU_ASSERT (descriptor.type == lyric_runtime::DataCellType::FIELD);
    auto id = ElementRef::getField(descriptor);
    if (id.type != lyric_runtime::DataCellType::I64)
        return false;
    if (std::numeric_limits<tu_uint32>::max() < id.data.i64)
        return false;
    auto idInt = static_cast<tu_uint32>(id.data.i64);

    // get children iterator
    symbol = object.findSymbol(lyric_common::SymbolPath({"Element", "$getChildrenIterator"}));
    TU_ASSERT (symbol.isValid());
    descriptor = segmentManager->resolveDescriptor(segment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    TU_ASSERT (descriptor.type == lyric_runtime::DataCellType::CALL);
    TU_ASSERT (descriptor.data.descriptor.assembly == currentCoro->peekSP()->getSegmentIndex());
    std::vector<lyric_runtime::DataCell> args{};

    if (!subroutineManager->callVirtual(this, descriptor.data.descriptor.value, args, currentCoro, status))
        return false;
    auto runInterpreterResult = m_interp->runSubinterpreter();
    if (runInterpreterResult.isStatus())
        return false;
    auto result = runInterpreterResult.getResult();
    TU_ASSERT (result.type == lyric_runtime::DataCellType::REF);
    auto *iterator = result.data.ref;

    // serialize each element in children member
    std::vector<lyric_serde::ValueAddress> children;
    while (iterator->iteratorValid()) {
        lyric_runtime::DataCell child;
        if (!iterator->iteratorNext(child))
            return false;
        tu_uint32 childOffset= lyric_runtime::serialize_value(child, state);
        if (childOffset == lyric_runtime::INVALID_ADDRESS_U32)
            return false;
        children.push_back(lyric_serde::ValueAddress(childOffset));
    }

    auto appendValueResult = state.appendValue(nsString.c_str(), static_cast<tu_uint32>(idInt), children);
    if (appendValueResult.isStatus())
        return lyric_runtime::INVALID_ADDRESS_U32;

    index = appendValueResult.getResult()->getAddress().getAddress();
    return true;
}

std::string
ElementRef::toString() const
{
    return absl::Substitute("<$0: ElementRef>", this);
}

void
ElementRef::setMembersReachable()
{
    for (auto &cell : m_fields) {
        if (cell.type == lyric_runtime::DataCellType::REF) {
            TU_ASSERT (cell.data.ref != nullptr);
            cell.data.ref->setReachable();
        }
    }
}

void
ElementRef::clearMembersReachable()
{
    for (auto &cell : m_fields) {
        if (cell.type == lyric_runtime::DataCellType::REF) {
            TU_ASSERT (cell.data.ref != nullptr);
            cell.data.ref->clearReachable();
        }
    }
}

tempo_utils::Status
element_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();
    const auto *vtable = frame.getVirtualTable();
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<ElementRef>(vtable, interp, state);
    currentCoro->pushData(ref);

    return lyric_runtime::InterpreterStatus::ok();
}