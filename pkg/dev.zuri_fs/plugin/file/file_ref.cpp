/* SPDX-License-Identifier: BSD-3-Clause */

#include <absl/strings/substitute.h>

#include <lyric_runtime/data_cell.h>
#include <lyric_runtime/string_ref.h>
#include <tempo_utils/log_stream.h>
#include <tempo_utils/unicode.h>

#include "file_ref.h"

FileRef::FileRef(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable),
      m_state(State::Initial)
{
}

FileRef::~FileRef()
{
    TU_LOG_V << "free FileRef" << FileRef::toString();
}

lyric_runtime::DataCell
FileRef::getField(const lyric_runtime::DataCell &field) const
{
    return {};
}

lyric_runtime::DataCell
FileRef::setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value)
{
    return {};
}

std::string
FileRef::toString() const
{
    return absl::Substitute("<$0: File $1>", this, m_path.string());
}

FileRef::State
FileRef::getState() const
{
    return m_state;
}

std::filesystem::path
FileRef::getPath() const
{
    return m_path;
}

void
FileRef::setPath(const std::filesystem::path &path)
{
    m_path = path;
}

tempo_utils::Status
FileRef::open(int flags, int mode, lyric_runtime::SystemScheduler *systemScheduler)
{
    switch (m_state) {
        case State::Initial:
            break;
        case State::Open:
            return lyric_runtime::InterpreterStatus::forCondition(
                lyric_runtime::InterpreterCondition::kRuntimeInvariant, "File is already open");
        case State::Closed:
            return lyric_runtime::InterpreterStatus::forCondition(
                lyric_runtime::InterpreterCondition::kRuntimeInvariant, "File has been closed");
        case State::Error:
            return m_status;
    }

    uv_fs_t req;
    auto ret = uv_fs_open(systemScheduler->systemLoop(), &req, m_path.c_str(), flags, mode, nullptr);
    if (ret < 0) {
        m_state = State::Error;
        m_status = lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant,
            "failed to open file '{}': {}", m_path.string(), uv_strerror(ret));
        return m_status;
    }
    m_file = req.file;
    m_state = State::Open;
    return {};
}

struct ReadContext {
    lyric_runtime::DataCell file;
    std::string data;
    uv_buf_t buf;
    ReadContext(const lyric_runtime::DataCell &file, size_t size)
    {
        this->file = file;
        data.resize(size);
        buf = uv_buf_init(data.data(), data.size());
    }
};

void
read_context_free(void *data)
{
    delete static_cast<ReadContext *>(data);
}

void
on_read_reachable(void *data)
{
    auto *ctx = static_cast<ReadContext *>(data);
    ctx->file.data.ref->setReachable();
}

static void
on_read_accept(
    lyric_runtime::Promise *promise,
    const lyric_runtime::Waiter *waiter,
    lyric_runtime::InterpreterState *state)
{
    auto *heapManager = state->heapManager();

    auto ret = waiter->req->result;
    if (ret >= 0) {
        auto *ctx = (ReadContext *) promise->getData();
        std::span bytes((const tu_uint8 *) ctx->buf.base, ret);
        auto data = heapManager->allocateBytes(bytes);
        promise->complete(data);
    } else {
        auto status = heapManager->allocateStatus(
            tempo_utils::StatusCode::kInternal, uv_strerror(ret));
        promise->reject(status);
    }
}

tempo_utils::Status
FileRef::readAsync(int size, AbstractRef *fut, lyric_runtime::SystemScheduler *systemScheduler)
{
    auto *ctx = new ReadContext(lyric_runtime::DataCell::forRef(this), size);

    lyric_runtime::PromiseOptions options;
    options.data = ctx;
    options.reachable = on_read_reachable;
    options.release = read_context_free;
    auto promise = lyric_runtime::Promise::create(on_read_accept, options);

    TU_RETURN_IF_NOT_OK (systemScheduler->registerRead(m_file, ctx->buf, promise));
    fut->prepareFuture(promise);

    return {};
}

tempo_utils::Status
FileRef::close(lyric_runtime::SystemScheduler *systemScheduler)
{
    switch (m_state) {
        case State::Initial:
            return {};
        case State::Open:
            break;
        case State::Closed:
            return lyric_runtime::InterpreterStatus::forCondition(
                lyric_runtime::InterpreterCondition::kRuntimeInvariant, "File is already closed");
        case State::Error:
            return m_status;
    }

    uv_fs_t req;
    auto ret = uv_fs_close(systemScheduler->systemLoop(), &req, m_file, nullptr);
    if (ret < 0) {
        m_state = State::Error;
        m_status = lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant,
            "failed to close file '{}': {}", m_path.string(), uv_strerror(ret));
        return m_status;
    }
    m_state = State::Closed;
    return {};
}

void
FileRef::setMembersReachable()
{
}

void
FileRef::clearMembersReachable()
{
}

tempo_utils::Status
fs_file_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    TU_ASSERT(vtable != nullptr);
    auto *currentCoro = state->currentCoro();
    auto ref = state->heapManager()->allocateRef<FileRef>(vtable);
    currentCoro->pushData(ref);
    return {};
}

tempo_utils::Status
fs_file_ctor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<FileRef *>(receiver.data.ref);

    TU_ASSERT (frame.numArguments() > 0);
    auto arg0 = frame.getArgument(0);
    TU_ASSERT (arg0.type == lyric_runtime::DataCellType::STRING);
    auto *str = arg0.data.str;
    std::string path(str->getStringData(), str->getStringSize());

    instance->setPath(path);

    return {};
}

tempo_utils::Status
fs_file_create(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();
    auto *systemScheduler = state->systemScheduler();

    auto &frame = currentCoro->currentCallOrThrow();

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<FileRef *>(receiver.data.ref);

    int flags = 0;
    int mode = 0644;

    lyric_runtime::DataCell canRead, canWrite;
    TU_RETURN_IF_NOT_OK (currentCoro->popData(canWrite));
    TU_RETURN_IF_NOT_OK (currentCoro->popData(canRead));
    if (canRead.data.b == true) {
        flags |= canWrite.data.b == true? UV_FS_O_RDWR : UV_FS_O_RDONLY;
    } else {
        flags |= canWrite.data.b == true? UV_FS_O_WRONLY : 0;
    }

    TU_RETURN_IF_NOT_OK (instance->open(flags, mode, systemScheduler));

    return {};
}

tempo_utils::Status
fs_file_open(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();
    auto *systemScheduler = state->systemScheduler();

    auto &frame = currentCoro->currentCallOrThrow();
    auto arg1 = frame.getArgument(1);
    TU_ASSERT (arg1.type == lyric_runtime::DataCellType::BOOL);
    auto arg2 = frame.getArgument(2);
    TU_ASSERT (arg2.type == lyric_runtime::DataCellType::BOOL);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<FileRef *>(receiver.data.ref);

    int flags = 0;
    int mode = 0644;

    lyric_runtime::DataCell canRead, canWrite;
    TU_RETURN_IF_NOT_OK (currentCoro->popData(canWrite));
    TU_RETURN_IF_NOT_OK (currentCoro->popData(canRead));
    if (canRead.data.b == true) {
        flags |= canWrite.data.b == true? UV_FS_O_RDWR : UV_FS_O_RDONLY;
    } else {
        flags |= canWrite.data.b == true? UV_FS_O_WRONLY : 0;
    }

    if (arg1.data.b == true) {
        flags |= UV_FS_O_CREAT;
    }
    if (arg2.data.b == true) {
        flags |= UV_FS_O_TRUNC;
    }

    TU_RETURN_IF_NOT_OK (instance->open(flags, mode, systemScheduler));

    return {};
}

tempo_utils::Status
fs_file_read(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();
    auto *systemScheduler = state->systemScheduler();

    auto &frame = currentCoro->currentCallOrThrow();

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<FileRef *>(receiver.data.ref);

    TU_ASSERT (frame.numArguments() == 1);
    auto arg0 = frame.getArgument(0);
    TU_ASSERT (arg0.type == lyric_runtime::DataCellType::I64);
    auto size = static_cast<size_t>(arg0.data.i64);

    lyric_runtime::DataCell *data;
    TU_RETURN_IF_NOT_OK (currentCoro->peekData(&data));
    TU_ASSERT (data->type == lyric_runtime::DataCellType::REF);
    auto *fut = data->data.ref;

    TU_RETURN_IF_NOT_OK (instance->readAsync(size, fut, systemScheduler));

    return {};
}

tempo_utils::Status
fs_file_write(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();
    auto *systemScheduler = state->systemScheduler();

    auto &frame = currentCoro->currentCallOrThrow();

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<FileRef *>(receiver.data.ref);

    TU_ASSERT (frame.numArguments() == 1);
    auto arg0 = frame.getArgument(0);
    TU_ASSERT (arg0.type == lyric_runtime::DataCellType::I64);
    auto size = static_cast<size_t>(arg0.data.i64);

    lyric_runtime::DataCell *data;
    TU_RETURN_IF_NOT_OK (currentCoro->peekData(&data));
    TU_ASSERT (data->type == lyric_runtime::DataCellType::REF);
    auto *fut = data->data.ref;

    TU_RETURN_IF_NOT_OK (instance->readAsync(size, fut, systemScheduler));

    return {};
}

tempo_utils::Status
fs_file_close(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();
    auto *systemScheduler = state->systemScheduler();

    auto &frame = currentCoro->currentCallOrThrow();

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<FileRef *>(receiver.data.ref);

    TU_RETURN_IF_NOT_OK (instance->close(systemScheduler));

    return {};
}
