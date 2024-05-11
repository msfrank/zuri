/* SPDX-License-Identifier: BSD-3-Clause */

#include <absl/strings/substitute.h>

#include <lyric_object/bytecode_iterator.h>
#include <lyric_runtime/interpreter_state.h>
#include <lyric_serde/patchset_change.h>
#include <lyric_serde/patchset_state.h>
#include <lyric_serde/patchset_value.h>
#include <tempo_utils/big_endian.h>
#include <tempo_utils/log_stream.h>
#include <tempo_utils/unicode.h>

#include "future_ref.h"
#include "port_ref.h"

PortRef::PortRef(
    const lyric_runtime::VirtualTable *vtable,
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state)
    : BaseRef(vtable),
      m_interp(interp),
      m_state(state)
{
    TU_ASSERT (m_interp != nullptr);
    TU_ASSERT (m_state != nullptr);
}

PortRef::PortRef(
    const lyric_runtime::VirtualTable *vtable,
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    std::shared_ptr<lyric_runtime::DuplexPort> port)
    : BaseRef(vtable),
      m_interp(interp),
      m_state(state),
      m_port(port)
{
    TU_ASSERT (m_interp != nullptr);
    TU_ASSERT (m_state != nullptr);
    TU_ASSERT (port != nullptr);
}

PortRef::~PortRef()
{
    TU_LOG_INFO << "free" << PortRef::toString();
}

lyric_runtime::DataCell
PortRef::getField(const lyric_runtime::DataCell &field) const
{
    return {};
}

lyric_runtime::DataCell
PortRef::setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value)
{
    return {};
}

std::string
PortRef::toString() const
{
    return absl::Substitute("<$0: PortRef>", this);
}

std::shared_ptr<lyric_runtime::DuplexPort>
PortRef::duplexPort()
{
    return m_port;
}

bool
PortRef::send(const lyric_serde::LyricPatchset &patchset)
{
    // if port is not attached then this is the null protocol, so drop the message
    if (!m_port)
        return true;
    // otherwise if a port is attached, then send the message through it
    m_port->send(patchset);
    return true;
}

bool
PortRef::waitForReceive(std::shared_ptr<lyric_runtime::Promise> promise, uv_async_t *async)
{
    if (m_port) {
        // if port is attached then signal the port that we are ready to receive
        m_port->readyToReceive(async);
    } else {
        // otherwise if this is the null protocol then signal a receive immediately
        promise->complete(lyric_runtime::DataCell::nil());
        uv_async_send(async);
    }

    return true;
}

static tempo_utils::Status
allocate_next_string(
    const lyric_serde::PatchsetWalker &patchset,
    const lyric_serde::ValueWalker &nextString,
    int stackBase,
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state)
{
    auto *heapManager = state->heapManager();
    auto utf8 = nextString.getString();
    return heapManager->loadStringOntoStack(utf8);
}

static tempo_utils::Status
allocate_next_attr(
    const lyric_serde::PatchsetWalker &patchset,
    const lyric_serde::ValueWalker &nextAttr,
    int stackBase,
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();
    auto *segment = currentCoro->peekSP();
    auto object = segment->getObject().getObject();

    auto *segmentManager = state->segmentManager();
    auto *subroutineManager = state->subroutineManager();
    auto *heapManager = state->heapManager();

    auto url = nextAttr.getAttrNamespace().getUrl();
    auto ns = heapManager->allocateUrl(url);

    lyric_runtime::InterpreterStatus status;

    //
    auto symbol = object.findSymbol(lyric_common::SymbolPath({"Operation", "$createAttr"}));
    TU_ASSERT (symbol.isValid());

    auto descriptor = segmentManager->resolveDescriptor(segment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    TU_ASSERT (descriptor.type == lyric_runtime::DataCellType::CALL);
    TU_ASSERT (descriptor.data.descriptor.assembly == currentCoro->peekSP()->getSegmentIndex());

    int index = stackBase + nextAttr.getIndex();
    if (currentCoro->dataStackSize() <= index)
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid attr value");
    lyric_runtime::DataCell id(static_cast<tu_int64>(nextAttr.getAttrResource().idValue));
    std::vector<lyric_runtime::DataCell> args{ns, id, currentCoro->peekData(index)};

    if (!subroutineManager->callStatic(descriptor.data.descriptor.value, args, currentCoro, status))
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "failed to create Attr");
    auto runInterpreterResult = interp->runSubinterpreter();
    if (runInterpreterResult.isStatus())
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "failed to create Attr");
    currentCoro->pushData(runInterpreterResult.getResult());

    return lyric_runtime::InterpreterStatus::ok();
}

static tempo_utils::Status
allocate_next_element(
    const lyric_serde::PatchsetWalker &patchset,
    const lyric_serde::ValueWalker &nextElement,
    int stackBase,
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();
    auto *segment = currentCoro->peekSP();
    auto object = segment->getObject().getObject();
    auto symbol = object.findSymbol(lyric_common::SymbolPath({"Operation", "$createElement"}));
    TU_ASSERT (symbol.isValid());

    auto *segmentManager = state->segmentManager();
    auto *subroutineManager = state->subroutineManager();

    lyric_runtime::InterpreterStatus status;

    auto descriptor = segmentManager->resolveDescriptor(segment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    TU_ASSERT (descriptor.type == lyric_runtime::DataCellType::CALL);
    TU_ASSERT (descriptor.data.descriptor.assembly == currentCoro->peekSP()->getSegmentIndex());

    std::vector<lyric_runtime::DataCell> args;
    for (tu_uint32 i = 0; i < nextElement.numElementChildren(); i++) {
        auto child = nextElement.getElementChild(i);
        auto index = stackBase + child.getIndex();
        if (currentCoro->dataStackSize() <= index)
            return lyric_runtime::InterpreterStatus::forCondition(
                lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid child value");
        args.push_back(currentCoro->peekData(index));
    }

    if (!subroutineManager->callStatic(descriptor.data.descriptor.value, args, currentCoro, status))
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "failed to create Element");
    auto runInterpreterResult = interp->runSubinterpreter();
    if (runInterpreterResult.isStatus())
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "failed to create Element");
    currentCoro->pushData(runInterpreterResult.getResult());

    return lyric_runtime::InterpreterStatus::ok();
}

static tempo_utils::Status
allocate_next_value(
    const lyric_serde::PatchsetWalker &patchset,
    const lyric_serde::ValueWalker &nextValue,
    int stackBase,
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state)
{
    if (!nextValue.isValid())
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "missing value");

    auto *currentCoro = state->currentCoro();

    switch (nextValue.getValueType()) {
        case lyric_serde::ValueType::Nil: {
            auto value = lyric_runtime::DataCell::nil();
            currentCoro->pushData(value);
            return lyric_runtime::InterpreterStatus::ok();
        }
        case lyric_serde::ValueType::Bool: {
            auto value = lyric_runtime::DataCell(nextValue.getBool());
            currentCoro->pushData(value);
            return lyric_runtime::InterpreterStatus::ok();
        }
        case lyric_serde::ValueType::Int64: {
            auto value = lyric_runtime::DataCell(nextValue.getInt64());
            currentCoro->pushData(value);
            return lyric_runtime::InterpreterStatus::ok();
        }
        case lyric_serde::ValueType::Float64: {
            auto value = lyric_runtime::DataCell(nextValue.getFloat64());
            currentCoro->pushData(value);
            return lyric_runtime::InterpreterStatus::ok();
        }
        case lyric_serde::ValueType::String: {
            return allocate_next_string(patchset, nextValue, stackBase, interp, state);
        }
        case lyric_serde::ValueType::Attr: {
            return allocate_next_attr(patchset, nextValue, stackBase, interp, state);
        }
        case lyric_serde::ValueType::Element: {
            return allocate_next_element(patchset, nextValue, stackBase, interp, state);
        }
        default:
            return lyric_runtime::InterpreterStatus::forCondition(
                lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid value");
    }
}

static tempo_utils::Result<lyric_runtime::DataCell>
allocate_append_operation(
    const lyric_serde::PatchsetWalker &patchset,
    const std::string &id,
    const lyric_serde::AppendOperationWalker &operation,
    int stackBase,
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();
    auto *segment = currentCoro->peekSP();
    auto object = segment->getObject().getObject();

    auto *segmentManager = state->segmentManager();
    auto *subroutineManager = state->subroutineManager();
    auto *heapManager = state->heapManager();

    auto utf8 = operation.getPath().pathView();
    auto path = heapManager->allocateString(utf8);

    lyric_runtime::InterpreterStatus status;

    auto symbol = object.findSymbol(lyric_common::SymbolPath({"AppendOperation", "$create"}));
    TU_ASSERT (symbol.isValid());
    auto descriptor = segmentManager->resolveDescriptor(segment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    TU_ASSERT (descriptor.type == lyric_runtime::DataCellType::CALL);
    TU_ASSERT (descriptor.data.descriptor.assembly == currentCoro->peekSP()->getSegmentIndex());
    int index = stackBase + operation.getValue().getIndex();

    if (currentCoro->dataStackSize() <= index)
        return lyric_runtime::InterpreterStatus::forCondition(
        lyric_runtime::InterpreterCondition::kRuntimeInvariant, "failed to create AppendOperation");
    auto value = currentCoro->peekData(index);

    std::vector<lyric_runtime::DataCell> args{path, value};
    if (!subroutineManager->callStatic(descriptor.data.descriptor.value, args, currentCoro, status))
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "failed to create AppendOperation");
    auto runInterpreterResult = interp->runSubinterpreter();
    if (runInterpreterResult.isStatus())
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "failed to create AppendOperation");
    return runInterpreterResult.getResult();
}

static tempo_utils::Result<lyric_runtime::DataCell>
allocate_insert_operation(
    const lyric_serde::PatchsetWalker &patchset,
    const std::string &id,
    const lyric_serde::InsertOperationWalker &operation,
    int stackBase,
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();
    auto *segment = currentCoro->peekSP();
    auto object = segment->getObject().getObject();

    auto *segmentManager = state->segmentManager();
    auto *subroutineManager = state->subroutineManager();
    auto *heapManager = state->heapManager();

    auto utf8 = operation.getPath().pathView();
    auto path = heapManager->allocateString(utf8);

    lyric_runtime::InterpreterStatus status;

    auto symbol = object.findSymbol(lyric_common::SymbolPath({"InsertOperation", "$create"}));
    TU_ASSERT (symbol.isValid());
    auto descriptor = segmentManager->resolveDescriptor(segment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    TU_ASSERT (descriptor.type == lyric_runtime::DataCellType::CALL);
    TU_ASSERT (descriptor.data.descriptor.assembly == currentCoro->peekSP()->getSegmentIndex());
    int index = stackBase + operation.getValue().getIndex();

    if (currentCoro->dataStackSize() <= index)
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "failed to create InsertOperation");
    auto value = currentCoro->peekData(index);

    std::vector<lyric_runtime::DataCell> args{
        path, lyric_runtime::DataCell(static_cast<tu_int64>(operation.getIndex())), value};
    if (!subroutineManager->callStatic(descriptor.data.descriptor.value, args, currentCoro, status))
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "failed to create InsertOperation");
    auto runInterpreterResult = interp->runSubinterpreter();
    if (runInterpreterResult.isStatus())
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "failed to create InsertOperation");
    return runInterpreterResult.getResult();
}

static tempo_utils::Result<lyric_runtime::DataCell>
allocate_update_operation(
    const lyric_serde::PatchsetWalker &patchset,
    const std::string &id,
    const lyric_serde::UpdateOperationWalker &operation,
    int stackBase,
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();
    auto *segment = currentCoro->peekSP();
    auto object = segment->getObject().getObject();

    auto *segmentManager = state->segmentManager();
    auto *subroutineManager = state->subroutineManager();
    auto *heapManager = state->heapManager();

    auto utf8 = operation.getPath().pathView();
    auto path = heapManager->allocateString(utf8);

    lyric_runtime::InterpreterStatus status;

    auto symbol = object.findSymbol(lyric_common::SymbolPath({"ReplaceOperation", "$create"}));
    TU_ASSERT (symbol.isValid());
    auto descriptor = segmentManager->resolveDescriptor(segment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    TU_ASSERT (descriptor.type == lyric_runtime::DataCellType::CALL);
    TU_ASSERT (descriptor.data.descriptor.assembly == currentCoro->peekSP()->getSegmentIndex());
    int index = stackBase + operation.getValue().getIndex();

    if (currentCoro->dataStackSize() <= index)
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "failed to create ReplaceOperation");
    auto value = currentCoro->peekData(index);

    std::vector<lyric_runtime::DataCell> args{path, value};
    if (!subroutineManager->callStatic(descriptor.data.descriptor.value, args, currentCoro, status))
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "failed to create ReplaceOperation");
    auto runInterpreterResult = interp->runSubinterpreter();
    if (runInterpreterResult.isStatus())
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "failed to create ReplaceOperation");
    return runInterpreterResult.getResult();
}

static tempo_utils::Result<lyric_runtime::DataCell>
allocate_replace_operation(
    const lyric_serde::PatchsetWalker &patchset,
    const std::string &id,
    const lyric_serde::ReplaceOperationWalker &operation,
    int stackBase,
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();
    auto *segment = currentCoro->peekSP();
    auto object = segment->getObject().getObject();

    auto *segmentManager = state->segmentManager();
    auto *subroutineManager = state->subroutineManager();
    auto *heapManager = state->heapManager();

    auto utf8 = operation.getPath().pathView();
    auto path = heapManager->allocateString(utf8);

    lyric_runtime::InterpreterStatus status;

    auto symbol = object.findSymbol(lyric_common::SymbolPath({"ReplaceOperation", "$create"}));
    TU_ASSERT (symbol.isValid());
    auto descriptor = segmentManager->resolveDescriptor(segment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    TU_ASSERT (descriptor.type == lyric_runtime::DataCellType::CALL);
    TU_ASSERT (descriptor.data.descriptor.assembly == currentCoro->peekSP()->getSegmentIndex());
    int index = stackBase + operation.getValue().getIndex();

    if (currentCoro->dataStackSize() <= index)
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "failed to create ReplaceOperation");
    auto value = currentCoro->peekData(index);

    std::vector<lyric_runtime::DataCell> args{path, value};
    if (!subroutineManager->callStatic(descriptor.data.descriptor.value, args, currentCoro, status))
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "failed to create ReplaceOperation");
    auto runInterpreterResult = interp->runSubinterpreter();
    if (runInterpreterResult.isStatus())
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "failed to create ReplaceOperation");
    return runInterpreterResult.getResult();
}

static tempo_utils::Result<lyric_runtime::DataCell>
allocate_emit_operation(
    const lyric_serde::PatchsetWalker &patchset,
    const std::string &id,
    const lyric_serde::EmitOperationWalker &operation,
    int stackBase,
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();
    auto *segment = currentCoro->peekSP();
    auto object = segment->getObject().getObject();
    auto symbol = object.findSymbol(lyric_common::SymbolPath({"EmitOperation", "$create"}));
    TU_ASSERT (symbol.isValid());

    auto *segmentManager = state->segmentManager();
    auto *subroutineManager = state->subroutineManager();

    lyric_runtime::InterpreterStatus status;

    auto descriptor = segmentManager->resolveDescriptor(segment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    TU_ASSERT (descriptor.type == lyric_runtime::DataCellType::CALL);
    TU_ASSERT (descriptor.data.descriptor.assembly == currentCoro->peekSP()->getSegmentIndex());

    auto index = stackBase + operation.getValue().getIndex();

    if (currentCoro->dataStackSize() <= index)
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "failed to create EmitOperation");

    std::vector<lyric_runtime::DataCell> args{currentCoro->peekData(index)};
    if (!subroutineManager->callStatic(descriptor.data.descriptor.value, args, currentCoro, status))
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "failed to create EmitOperation");
    auto runInterpreterResult = interp->runSubinterpreter();
    if (runInterpreterResult.isStatus())
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "failed to create EmitOperation");
    return runInterpreterResult.getResult();
}

static tempo_utils::Result<lyric_runtime::DataCell>
allocate_operation(
    const lyric_serde::PatchsetWalker &patchset,
    const lyric_serde::ChangeWalker &change,
    int stackBase,
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state)
{
    auto id = change.getId();

    switch (change.getOperationType()) {
        case lyric_serde::ChangeOperation::AppendOperation:
            return allocate_append_operation(patchset, id, change.getAppendOperation(),
                stackBase, interp, state);
        case lyric_serde::ChangeOperation::InsertOperation:
            return allocate_insert_operation(patchset, id, change.getInsertOperation(),
                stackBase, interp, state);
        case lyric_serde::ChangeOperation::UpdateOperation:
            return allocate_update_operation(patchset, id, change.getUpdateOperation(),
                stackBase, interp, state);
        case lyric_serde::ChangeOperation::ReplaceOperation:
            return allocate_replace_operation(patchset, id, change.getReplaceOperation(),
                stackBase, interp, state);
        case lyric_serde::ChangeOperation::EmitOperation:
            return allocate_emit_operation(patchset, id, change.getEmitOperation(),
                stackBase, interp, state);
        default:
            return lyric_runtime::InterpreterStatus::forCondition(
                lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid operation type");
    }
}

void
PortRef::setMembersReachable()
{
    for (auto &value : m_values) {
        if (value.type == lyric_runtime::DataCellType::REF)
            value.data.ref->setReachable();
    }
    //if (m_fut)
    //    m_fut->setReachable();
}

void
PortRef::clearMembersReachable()
{
    for (auto &value : m_values) {
        if (value.type == lyric_runtime::DataCellType::REF)
            value.data.ref->clearReachable();
    }
    //if (m_fut)
    //    m_fut->clearReachable();
}

tempo_utils::Status
port_send(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 1);
    const auto &arg0 = frame.getArgument(0);
    if (arg0.type != lyric_runtime::DataCellType::REF)
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid operation");
    auto *op = arg0.data.ref;

    lyric_serde::PatchsetState patchsetState;
    tu_uint32 ignored;
    if (!op->serializeValue(patchsetState, ignored))
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "failed to serialize operation");
    auto toPatchsetResult = patchsetState.toPatchset();
    if (toPatchsetResult.isStatus())
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "failed to serialize patchset");
    auto patchset = toPatchsetResult.getResult();

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<PortRef *>(receiver.data.ref);
    auto ret = instance->send(patchset);
    currentCoro->pushData(lyric_runtime::DataCell(ret));

    return lyric_runtime::InterpreterStatus::ok();
}

struct ResolveData {
    std::shared_ptr<lyric_runtime::DuplexPort> port;
    lyric_serde::LyricPatchset patchset;
    lyric_runtime::DataCell ref;
};


static void
on_resolve_patchset(
    lyric_runtime::Promise *promise,
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state)
{
    auto *data = static_cast<ResolveData *>(promise->getData());

    // take a new reference to patchset and delete the data struct
    auto patchset = data->patchset.getPatchset();
    delete data;

    TU_ASSERT (patchset.numChanges() == 1);
    auto change = patchset.getChange(0);

    //
    auto *currentCoro = state->currentCoro();
    int stackBase = currentCoro->dataStackSize();

    //
    for (uint32_t i = 0; i < patchset.numValues(); i++) {
        const auto value = patchset.getValue(i);
        TU_RAISE_IF_NOT_OK (allocate_next_value(patchset, value, stackBase, interp, state));
    }

    lyric_runtime::DataCell result;
    TU_ASSIGN_OR_RAISE (result, allocate_operation(patchset, change, stackBase, interp, state));
    promise->complete(result);
    currentCoro->resizeDataStack(stackBase);
}

static void
on_async_complete(lyric_runtime::Promise *promise)
{
    auto *data = static_cast<ResolveData *>(promise->getData());
    TU_ASSERT (data->port->hasPending());
    data->patchset = data->port->nextPending();
}

tempo_utils::Status
port_receive(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();
    auto *segmentManager = state->segmentManager();
    auto *scheduler = state->systemScheduler();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<PortRef *>(receiver.data.ref);

    //
    auto *segment = currentCoro->peekSP();
    auto object = segment->getObject().getObject();
    auto symbol = object.findSymbol(lyric_common::SymbolPath({"Future"}));
    TU_ASSERT (symbol.isValid());
    lyric_runtime::InterpreterStatus status;
    auto descriptor = segmentManager->resolveDescriptor(segment,
        symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
    TU_ASSERT (descriptor.type == lyric_runtime::DataCellType::CLASS);
    const auto *vtable = segmentManager->resolveClassVirtualTable(descriptor, status);
    TU_ASSERT(vtable != nullptr);

    // create a new future to wait for receive result
    auto ref = state->heapManager()->allocateRef<FutureRef>(vtable);
    currentCoro->pushData(ref);
    auto *fut = ref.data.ref;

    //
    auto *data = static_cast<ResolveData *>(std::malloc(sizeof(ResolveData)));
    data->port = instance->duplexPort();

    //
    lyric_runtime::PromiseOptions options;
    options.adapt = on_resolve_patchset;
    options.release = std::free;
    options.data = data;
    auto promise = lyric_runtime::Promise::create(on_async_complete, options);

    // register a waiter bound to the current task
    uv_async_t *async = nullptr;
    scheduler->registerAsync(&async, promise);

    // attach the waiter to the future
    fut->prepareFuture(promise);

    // set port to await a message from the remote side
    instance->waitForReceive(promise, async);

    return lyric_runtime::InterpreterStatus::ok();
}
