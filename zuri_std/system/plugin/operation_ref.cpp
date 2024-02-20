/* SPDX-License-Identifier: BSD-3-Clause */

#include <absl/strings/substitute.h>

#include <lyric_object/bytecode_iterator.h>
#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/serialize_value.h>
#include <lyric_serde/patchset_change.h>
#include <tempo_utils/big_endian.h>
#include <tempo_utils/log_stream.h>

#include "operation_ref.h"

OperationRef::OperationRef(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable)
{
    m_fields.resize(vtable->getLayoutTotal());
}

OperationRef::~OperationRef()
{
    TU_LOG_INFO << "free" << OperationRef::toString();
}

lyric_runtime::DataCell
OperationRef::getField(const lyric_runtime::DataCell &field) const
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
OperationRef::setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value)
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

std::string
OperationRef::toString() const
{
    return absl::Substitute("<$0: OperationRef>", this);
}

void
OperationRef::setMembersReachable()
{
    for (auto &cell : m_fields) {
        if (cell.type == lyric_runtime::DataCellType::REF) {
            TU_ASSERT (cell.data.ref != nullptr);
            cell.data.ref->setReachable();
        }
    }
}

void
OperationRef::clearMembersReachable()
{
    for (auto &cell : m_fields) {
        if (cell.type == lyric_runtime::DataCellType::REF) {
            TU_ASSERT (cell.data.ref != nullptr);
            cell.data.ref->clearReachable();
        }
    }
}

AppendOperationRef::AppendOperationRef(
    const lyric_runtime::VirtualTable *vtable,
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state)
    : OperationRef(vtable),
      m_interp(interp),
      m_state(state)
{
    TU_ASSERT (m_interp != nullptr);
    TU_ASSERT (m_state != nullptr);
}

bool
AppendOperationRef::serializeValue(lyric_serde::PatchsetState &state, tu_uint32 &ignored)
{
    auto *currentCoro = m_state->currentCoro();
    auto *segment = currentCoro->peekSP();
    auto object = segment->getObject().getObject();

    auto *segmentManager = m_state->segmentManager();

    lyric_runtime::InterpreterStatus status;

    auto symbol = object.findSymbol(lyric_common::SymbolPath({"AppendOperation", "path"}));
    TU_ASSERT (symbol.isValid());
    auto descriptor = segmentManager->resolveDescriptor(segment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    TU_ASSERT (descriptor.type == lyric_runtime::DataCellType::FIELD);
    auto path = OperationRef::getField(descriptor);
    if (path.type != lyric_runtime::DataCellType::REF)
        return false;
    std::string pathString;
    if (!path.data.ref->utf8Value(pathString))
        return false;
    if (pathString.empty())
        return false;

    symbol = object.findSymbol(lyric_common::SymbolPath({"AppendOperation", "value"}));
    TU_ASSERT (symbol.isValid());
    descriptor = segmentManager->resolveDescriptor(segment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    TU_ASSERT (descriptor.type == lyric_runtime::DataCellType::FIELD);
    auto value = OperationRef::getField(descriptor);

    auto valueOffset = lyric_runtime::serialize_value(value, state);
    if (valueOffset == lyric_runtime::INVALID_ADDRESS_U32)
        return false;

    auto appendChangeResult = state.appendChange("");
    if (appendChangeResult.isStatus())
        return false;
    auto *change = appendChangeResult.getResult();

    auto operationPath = lyric_serde::OperationPath::fromString(pathString);
    lyric_serde::ValueAddress valueAddress(valueOffset);
    change->setAppendOperation(operationPath, valueAddress);

    return true;
}

InsertOperationRef::InsertOperationRef(
    const lyric_runtime::VirtualTable *vtable,
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state)
    : OperationRef(vtable),
      m_interp(interp),
      m_state(state)
{
    TU_ASSERT (m_interp != nullptr);
    TU_ASSERT (m_state != nullptr);
}

bool
InsertOperationRef::serializeValue(lyric_serde::PatchsetState &state, tu_uint32 &ignored)
{
    auto *currentCoro = m_state->currentCoro();
    auto *segment = currentCoro->peekSP();
    auto object = segment->getObject().getObject();

    auto *segmentManager = m_state->segmentManager();

    lyric_runtime::InterpreterStatus status;

    auto symbol = object.findSymbol(lyric_common::SymbolPath({"InsertOperation", "path"}));
    TU_ASSERT (symbol.isValid());
    auto descriptor = segmentManager->resolveDescriptor(segment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    TU_ASSERT (descriptor.type == lyric_runtime::DataCellType::FIELD);
    auto path = OperationRef::getField(descriptor);
    if (path.type != lyric_runtime::DataCellType::REF)
        return false;
    std::string pathString;
    if (!path.data.ref->utf8Value(pathString))
        return false;
    if (pathString.empty())
        return false;

    symbol = object.findSymbol(lyric_common::SymbolPath({"InsertOperation", "index"}));
    TU_ASSERT (symbol.isValid());
    descriptor = segmentManager->resolveDescriptor(segment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    TU_ASSERT (descriptor.type == lyric_runtime::DataCellType::FIELD);
    auto index = OperationRef::getField(descriptor);
    if (index.type != lyric_runtime::DataCellType::I64)
        return false;
    if (std::numeric_limits<tu_uint32>::max() < index.data.i64)
        return false;
    auto indexInt = static_cast<tu_uint32>(index.data.i64);

    symbol = object.findSymbol(lyric_common::SymbolPath({"InsertOperation", "value"}));
    TU_ASSERT (symbol.isValid());
    descriptor = segmentManager->resolveDescriptor(segment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    TU_ASSERT (descriptor.type == lyric_runtime::DataCellType::FIELD);
    auto value = OperationRef::getField(descriptor);

    auto valueOffset = lyric_runtime::serialize_value(value, state);
    if (valueOffset == lyric_runtime::INVALID_ADDRESS_U32)
        return false;

    auto appendChangeResult = state.appendChange("");
    if (appendChangeResult.isStatus())
        return false;
    auto *change = appendChangeResult.getResult();

    auto operationPath = lyric_serde::OperationPath::fromString(pathString);
    lyric_serde::ValueAddress valueAddress(valueOffset);
    change->setInsertOperation(operationPath, indexInt, valueAddress);

    return true;
}

UpdateOperationRef::UpdateOperationRef(
    const lyric_runtime::VirtualTable *vtable,
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state)
    : OperationRef(vtable),
      m_interp(interp),
      m_state(state)
{
    TU_ASSERT (m_interp != nullptr);
    TU_ASSERT (m_state != nullptr);
}

bool
UpdateOperationRef::serializeValue(lyric_serde::PatchsetState &state, tu_uint32 &ignored)
{
    auto *currentCoro = m_state->currentCoro();
    auto *segment = currentCoro->peekSP();
    auto object = segment->getObject().getObject();

    auto *segmentManager = m_state->segmentManager();

    lyric_runtime::InterpreterStatus status;

    auto symbol = object.findSymbol(lyric_common::SymbolPath({"UpdateOperation", "path"}));
    TU_ASSERT (symbol.isValid());
    auto descriptor = segmentManager->resolveDescriptor(segment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    TU_ASSERT (descriptor.type == lyric_runtime::DataCellType::FIELD);
    auto path = OperationRef::getField(descriptor);
    if (path.type != lyric_runtime::DataCellType::REF)
        return false;
    std::string pathString;
    if (!path.data.ref->utf8Value(pathString))
        return false;
    if (pathString.empty())
        return false;

    symbol = object.findSymbol(lyric_common::SymbolPath({"InsertOperation", "ns"}));
    TU_ASSERT (symbol.isValid());
    descriptor = segmentManager->resolveDescriptor(segment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    TU_ASSERT (descriptor.type == lyric_runtime::DataCellType::FIELD);
    auto ns = OperationRef::getField(descriptor);
    if (ns.type != lyric_runtime::DataCellType::REF)
        return false;
    tempo_utils::Url nsUrl;
    if (!ns.data.ref->uriValue(nsUrl))
        return false;
    if (!nsUrl.isValid())
        return false;
    auto nsString = nsUrl.toString();

    symbol = object.findSymbol(lyric_common::SymbolPath({"InsertOperation", "id"}));
    TU_ASSERT (symbol.isValid());
    descriptor = segmentManager->resolveDescriptor(segment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    TU_ASSERT (descriptor.type == lyric_runtime::DataCellType::FIELD);
    auto id = OperationRef::getField(descriptor);
    if (id.type != lyric_runtime::DataCellType::I64)
        return false;
    if (std::numeric_limits<tu_uint32>::max() < id.data.i64)
        return false;
    auto idInt = static_cast<tu_uint32>(id.data.i64);

    symbol = object.findSymbol(lyric_common::SymbolPath({"UpdateOperation", "value"}));
    TU_ASSERT (symbol.isValid());
    descriptor = segmentManager->resolveDescriptor(segment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    TU_ASSERT (descriptor.type == lyric_runtime::DataCellType::FIELD);
    auto value = OperationRef::getField(descriptor);

    auto valueOffset = lyric_runtime::serialize_value(value, state);
    if (valueOffset == lyric_runtime::INVALID_ADDRESS_U32)
        return false;

    auto appendChangeResult = state.appendChange("");
    if (appendChangeResult.isStatus())
        return false;
    auto *change = appendChangeResult.getResult();

    auto operationPath = lyric_serde::OperationPath::fromString(pathString);
    lyric_serde::ValueAddress valueAddress(valueOffset);
    change->setUpdateOperation(operationPath, nsString.c_str(), idInt, valueAddress);

    return true;
}

ReplaceOperationRef::ReplaceOperationRef(
    const lyric_runtime::VirtualTable *vtable,
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state)
    : OperationRef(vtable),
      m_interp(interp),
      m_state(state)
{
    TU_ASSERT (m_interp != nullptr);
    TU_ASSERT (m_state != nullptr);
}

bool
ReplaceOperationRef::serializeValue(lyric_serde::PatchsetState &state, tu_uint32 &ignored)
{
    auto *currentCoro = m_state->currentCoro();
    auto *segment = currentCoro->peekSP();
    auto object = segment->getObject().getObject();

    auto *segmentManager = m_state->segmentManager();

    lyric_runtime::InterpreterStatus status;

    auto symbol = object.findSymbol(lyric_common::SymbolPath({"ReplaceOperation", "path"}));
    TU_ASSERT (symbol.isValid());
    auto descriptor = segmentManager->resolveDescriptor(segment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    TU_ASSERT (descriptor.type == lyric_runtime::DataCellType::FIELD);
    auto path = OperationRef::getField(descriptor);
    if (path.type != lyric_runtime::DataCellType::REF)
        return false;
    std::string pathString;
    if (!path.data.ref->utf8Value(pathString))
        return false;
    if (pathString.empty())
        return false;

    symbol = object.findSymbol(lyric_common::SymbolPath({"ReplaceOperation", "value"}));
    TU_ASSERT (symbol.isValid());
    descriptor = segmentManager->resolveDescriptor(segment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    TU_ASSERT (descriptor.type == lyric_runtime::DataCellType::FIELD);
    auto value = OperationRef::getField(descriptor);

    auto valueOffset = lyric_runtime::serialize_value(value, state);
    if (valueOffset == lyric_runtime::INVALID_ADDRESS_U32)
        return false;

    auto appendChangeResult = state.appendChange("");
    if (appendChangeResult.isStatus())
        return false;
    auto *change = appendChangeResult.getResult();

    auto operationPath = lyric_serde::OperationPath::fromString(pathString);
    lyric_serde::ValueAddress valueAddress(valueOffset);
    change->setReplaceOperation(operationPath, valueAddress);

    return true;
}

EmitOperationRef::EmitOperationRef(
    const lyric_runtime::VirtualTable *vtable,
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state)
    : OperationRef(vtable),
      m_interp(interp),
      m_state(state)
{
    TU_ASSERT (m_interp != nullptr);
    TU_ASSERT (m_state != nullptr);
}

bool
EmitOperationRef::serializeValue(lyric_serde::PatchsetState &state, tu_uint32 &ignored)
{
    auto *currentCoro = m_state->currentCoro();
    auto *segment = currentCoro->peekSP();
    auto object = segment->getObject().getObject();

    auto *segmentManager = m_state->segmentManager();

    lyric_runtime::InterpreterStatus status;

    auto symbol = object.findSymbol(lyric_common::SymbolPath({"EmitOperation", "value"}));
    TU_ASSERT (symbol.isValid());
    auto descriptor = segmentManager->resolveDescriptor(segment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    TU_ASSERT (descriptor.type == lyric_runtime::DataCellType::FIELD);
    auto value = OperationRef::getField(descriptor);

    auto valueOffset = lyric_runtime::serialize_value(value, state);
    if (valueOffset == lyric_runtime::INVALID_ADDRESS_U32)
        return false;

    auto appendChangeResult = state.appendChange("");
    if (appendChangeResult.isStatus())
        return false;
    auto *change = appendChangeResult.getResult();

    change->setEmitOperation(lyric_serde::ValueAddress(valueOffset));

    return true;
}

tempo_utils::Status
append_operation_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();
    const auto *vtable = frame.getVirtualTable();
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<AppendOperationRef>(vtable, interp, state);
    currentCoro->pushData(ref);

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
insert_operation_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();
    const auto *vtable = frame.getVirtualTable();
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<InsertOperationRef>(vtable, interp, state);
    currentCoro->pushData(ref);

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
update_operation_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();
    const auto *vtable = frame.getVirtualTable();
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<UpdateOperationRef>(vtable, interp, state);
    currentCoro->pushData(ref);

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
replace_operation_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();
    const auto *vtable = frame.getVirtualTable();
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<ReplaceOperationRef>(vtable, interp, state);
    currentCoro->pushData(ref);

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
emit_operation_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();
    const auto *vtable = frame.getVirtualTable();
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<EmitOperationRef>(vtable, interp, state);
    currentCoro->pushData(ref);

    return lyric_runtime::InterpreterStatus::ok();
}