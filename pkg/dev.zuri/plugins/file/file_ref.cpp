/* SPDX-License-Identifier: BSD-3-Clause */

#include <absl/strings/substitute.h>

#include <lyric_runtime/data_cell.h>
#include <lyric_runtime/string_ref.h>
#include <tempo_utils/log_stream.h>
#include <tempo_utils/unicode.h>

#include "file_ref.h"

#include <lyric_runtime/bytes_ref.h>

FileRef::FileRef(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable),
      m_state(State::Initial)
{
}

FileRef::~FileRef()
{
    TU_LOG_V << "free FileRef" << FileRef::toString();
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
    } else {
        m_file = ret;
        m_state = State::Open;
    }
    uv_fs_req_cleanup(&req);

    return m_status;
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

struct WriteContext {
    FileRef *file;
    lyric_runtime::BytesRef *bytes;
    uv_buf_t buf;
    WriteContext(FileRef *file, lyric_runtime::BytesRef *bytes)
    {
        this->file = file;
        this->bytes = bytes;
        auto *data = (char *) bytes->getBytesData();
        auto size = bytes->getBytesSize();
        buf = uv_buf_init(data, size);
    }
};

void
write_context_free(void *data)
{
    delete static_cast<WriteContext *>(data);
}

void
on_write_reachable(void *data)
{
    auto *ctx = static_cast<WriteContext *>(data);
    ctx->file->setReachable();
    ctx->bytes->setReachable();
}

static void
on_write_accept(
    lyric_runtime::Promise *promise,
    const lyric_runtime::Waiter *waiter,
    lyric_runtime::InterpreterState *state)
{
    auto *heapManager = state->heapManager();

    auto ret = waiter->req->result;
    if (ret >= 0) {
        promise->complete(lyric_runtime::DataCell(static_cast<tu_int64>(ret)));
    } else {
        auto status = heapManager->allocateStatus(
            tempo_utils::StatusCode::kInternal, uv_strerror(ret));
        promise->reject(status);
    }
}

tempo_utils::Status
FileRef::writeAsync(
    lyric_runtime::BytesRef *bytes,
    tu_int64 offset,
    AbstractRef *fut,
    lyric_runtime::SystemScheduler *systemScheduler)
{
    auto *ctx = new WriteContext(this, bytes);

    lyric_runtime::PromiseOptions options;
    options.data = ctx;
    options.reachable = on_write_reachable;
    options.release = write_context_free;
    auto promise = lyric_runtime::Promise::create(on_write_accept, options);

    TU_RETURN_IF_NOT_OK (systemScheduler->registerWrite(m_file, ctx->buf, promise, offset));
    fut->prepareFuture(promise);

    return {};
}

tempo_utils::Status
FileRef::truncateAsync(tu_int64 size, AbstractRef *fut, lyric_runtime::SystemScheduler *systemScheduler)
{
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
    } else {
        m_state = State::Closed;
    }
    uv_fs_req_cleanup(&req);

    return m_status;
}

tempo_utils::Status
fs_file_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
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
    std::string_view s(str->getStringData(), str->getStringSize());

    std::filesystem::path path(s);
    path = std::filesystem::absolute(path);
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
    TU_ASSERT (frame.numArguments() == 4);
    auto arg2 = frame.getArgument(2);
    TU_ASSERT (arg2.type == lyric_runtime::DataCellType::BOOL);
    auto arg3 = frame.getArgument(3);
    TU_ASSERT (arg3.type == lyric_runtime::DataCellType::BOOL);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<FileRef *>(receiver.data.ref);

    int flags = UV_FS_O_CREAT | UV_FS_O_EXCL;
    int mode = 0;

    lyric_runtime::DataCell permissions;
    TU_RETURN_IF_NOT_OK (currentCoro->popData(permissions));
    TU_ASSERT (permissions.type == lyric_runtime::DataCellType::I64);
    mode = static_cast<int>(permissions.data.i64);

    lyric_runtime::DataCell canRead, canWrite;
    TU_RETURN_IF_NOT_OK (currentCoro->popData(canWrite));
    TU_ASSERT (canWrite.type == lyric_runtime::DataCellType::BOOL);
    TU_RETURN_IF_NOT_OK (currentCoro->popData(canRead));
    TU_ASSERT (canRead.type == lyric_runtime::DataCellType::BOOL);

    if (canRead.data.b == true) {
        flags |= canWrite.data.b == true? UV_FS_O_RDWR : UV_FS_O_RDONLY;
    } else {
        flags |= canWrite.data.b == true? UV_FS_O_WRONLY : 0;
    }

    if (arg2.data.b == true) {
        flags |= UV_FS_O_TRUNC;
    }
    if (arg3.data.b == true) {
        flags |= UV_FS_O_APPEND;
    }

    auto status = instance->open(flags, mode, systemScheduler);
    if (status.notOk()) {
        auto *heapManager = state->heapManager();
        auto statusRef = heapManager->allocateStatus(status.getStatusCode(), status.getMessage());
        TU_RETURN_IF_NOT_OK (currentCoro->pushData(statusRef));
    } else {
        TU_RETURN_IF_NOT_OK (currentCoro->pushData(receiver));
    }

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
    TU_ASSERT (frame.numArguments() == 4);
    auto arg1 = frame.getArgument(1);
    TU_ASSERT (arg1.type == lyric_runtime::DataCellType::BOOL);
    auto arg2 = frame.getArgument(2);
    TU_ASSERT (arg2.type == lyric_runtime::DataCellType::BOOL);
    auto arg3 = frame.getArgument(3);
    TU_ASSERT (arg3.type == lyric_runtime::DataCellType::BOOL);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<FileRef *>(receiver.data.ref);

    int flags = 0;
    int mode = 0;

    lyric_runtime::DataCell canRead, canWrite;
    TU_RETURN_IF_NOT_OK (currentCoro->popData(canWrite));
    TU_RETURN_IF_NOT_OK (currentCoro->popData(canRead));
    if (canRead.data.b == true) {
        flags |= canWrite.data.b == true? UV_FS_O_RDWR : UV_FS_O_RDONLY;
    } else {
        flags |= canWrite.data.b == true? UV_FS_O_WRONLY : 0;
    }

    if (arg1.data.b == true) {
        flags |= UV_FS_O_TRUNC;
    }
    if (arg2.data.b == true) {
        flags |= UV_FS_O_APPEND;
    }
    if (arg3.data.b == true) {
        flags |= UV_FS_O_NOFOLLOW;
    }

    auto status = instance->open(flags, mode, systemScheduler);
    if (status.notOk()) {
        auto *heapManager = state->heapManager();
        auto statusRef = heapManager->allocateStatus(status.getStatusCode(), status.getMessage());
        TU_RETURN_IF_NOT_OK (currentCoro->pushData(statusRef));
    } else {
        TU_RETURN_IF_NOT_OK (currentCoro->pushData(receiver));
    }

    return {};
}

tempo_utils::Status
fs_file_open_or_create(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();
    auto *systemScheduler = state->systemScheduler();

    auto &frame = currentCoro->currentCallOrThrow();
    TU_ASSERT (frame.numArguments() == 5);
    auto arg2 = frame.getArgument(2);
    TU_ASSERT (arg2.type == lyric_runtime::DataCellType::BOOL);
    auto arg3 = frame.getArgument(3);
    TU_ASSERT (arg3.type == lyric_runtime::DataCellType::BOOL);
    auto arg4 = frame.getArgument(4);
    TU_ASSERT (arg4.type == lyric_runtime::DataCellType::BOOL);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<FileRef *>(receiver.data.ref);

    int flags = UV_FS_O_CREAT;
    int mode = 0;

    lyric_runtime::DataCell permissions;
    TU_RETURN_IF_NOT_OK (currentCoro->popData(permissions));
    TU_ASSERT (permissions.type == lyric_runtime::DataCellType::I64);
    mode = static_cast<int>(permissions.data.i64);

    lyric_runtime::DataCell canRead, canWrite;
    TU_RETURN_IF_NOT_OK (currentCoro->popData(canWrite));
    TU_ASSERT (canWrite.type == lyric_runtime::DataCellType::BOOL);
    TU_RETURN_IF_NOT_OK (currentCoro->popData(canRead));
    TU_ASSERT (canRead.type == lyric_runtime::DataCellType::BOOL);

    if (canRead.data.b == true) {
        flags |= canWrite.data.b == true? UV_FS_O_RDWR : UV_FS_O_RDONLY;
    } else {
        flags |= canWrite.data.b == true? UV_FS_O_WRONLY : 0;
    }

    if (arg2.data.b == true) {
        flags |= UV_FS_O_TRUNC;
    }
    if (arg3.data.b == true) {
        flags |= UV_FS_O_APPEND;
    }
    if (arg4.data.b == true) {
        flags |= UV_FS_O_NOFOLLOW;
    }

    auto status = instance->open(flags, mode, systemScheduler);
    if (status.notOk()) {
        auto *heapManager = state->heapManager();
        auto statusRef = heapManager->allocateStatus(status.getStatusCode(), status.getMessage());
        TU_RETURN_IF_NOT_OK (currentCoro->pushData(statusRef));
    } else {
        TU_RETURN_IF_NOT_OK (currentCoro->pushData(receiver));
    }

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
    auto *heapManager = state->heapManager();

    auto &frame = currentCoro->currentCallOrThrow();

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<FileRef *>(receiver.data.ref);

    TU_ASSERT (frame.numArguments() == 1);
    auto arg0 = frame.getArgument(0);
    TU_ASSERT (arg0.type == lyric_runtime::DataCellType::I64);

    lyric_runtime::DataCell *data;
    TU_RETURN_IF_NOT_OK (currentCoro->peekData(&data));
    TU_ASSERT (data->type == lyric_runtime::DataCellType::REF);
    auto *fut = data->data.ref;

    if (arg0.data.i64 < 0) {
        auto status = heapManager->allocateStatus(tempo_utils::StatusCode::kInvalidArgument,
            "invalid argument maxBytes; maxBytes cannot be negative");
        auto promise = lyric_runtime::Promise::rejected(status);
        fut->prepareFuture(promise);
        return {};
    }
    auto size = static_cast<size_t>(arg0.data.i64);

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
    auto *heapManager = state->heapManager();

    auto &frame = currentCoro->currentCallOrThrow();

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<FileRef *>(receiver.data.ref);

    TU_ASSERT (frame.numArguments() == 2);
    auto arg0 = frame.getArgument(0);
    TU_ASSERT (arg0.type == lyric_runtime::DataCellType::BYTES);
    auto *bytes = arg0.data.bytes;
    auto arg1 = frame.getArgument(1);
    TU_ASSERT (arg1.type == lyric_runtime::DataCellType::I64);
    auto offset = arg1.data.i64;

    lyric_runtime::DataCell *data;
    TU_RETURN_IF_NOT_OK (currentCoro->peekData(&data));
    TU_ASSERT (data->type == lyric_runtime::DataCellType::REF);
    auto *fut = data->data.ref;

    // ensure any negative offset is set to -1 specifically
    if (offset < -1) {
        offset = -1;
    }

    // reject the future if bytes argument is empty
    auto size = bytes->getBytesSize();
    if (size == 0) {
        auto status = heapManager->allocateStatus(tempo_utils::StatusCode::kInvalidArgument,
            "invalid argument bytes; bytes is empty");
        auto promise = lyric_runtime::Promise::rejected(status);
        fut->prepareFuture(promise);
        return {};
    }

    TU_RETURN_IF_NOT_OK (instance->writeAsync(bytes, offset, fut, systemScheduler));

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

    auto status = instance->close(systemScheduler);
    if (status.notOk()) {
        auto *heapManager = state->heapManager();
        auto statusRef = heapManager->allocateStatus(status.getStatusCode(), status.getMessage());
        TU_RETURN_IF_NOT_OK (currentCoro->pushData(statusRef));
    } else {
        TU_RETURN_IF_NOT_OK (currentCoro->pushData(lyric_runtime::DataCell::undef()));
    }

    return {};
}
