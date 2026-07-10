
#include <absl/strings/substitute.h>

#include <lyric_runtime/string_ref.h>

#include "url_ref.h"

UrlRef::UrlRef(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable)
{
}

UrlRef::~UrlRef()
{
    TU_LOG_INFO << "free" << UrlRef::toString();
}

tu_uint64
UrlRef::getTypeTag() const
{
    return type_tag();
}

tempo_utils::Url
UrlRef::getUrl() const
{
    return m_url;
}

void
UrlRef::setUrl(const tempo_utils::Url &url)
{
    m_url = url;
}

std::string
UrlRef::toString() const
{
    return absl::Substitute("<$0: Url \"$1\">", this, m_url.toString());
}

tempo_utils::Status
std_url_alloc(
    lyric_runtime::BytecodeInterpreter *,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    TU_ASSERT(vtable != nullptr);
    auto *currentCoro = state->currentCoro();
    auto ref = state->heapManager()->allocateRef<UrlRef>(vtable);
    currentCoro->pushData(ref);
    return {};
}

tempo_utils::Status
std_url_equality_equals(
    lyric_runtime::BytecodeInterpreter *,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT (frame.numArguments() == 2);
    auto arg0 = frame.getArgument(0);
    UrlRef *lhs;
    TU_ASSERT(arg0.castRef(lhs));
    auto arg1 = frame.getArgument(0);
    UrlRef *rhs;
    TU_ASSERT(arg1.castRef(rhs));

    auto result = lyric_runtime::Operand::fromBool(lhs->getUrl() == rhs->getUrl());
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(result));

    return {};
}

tempo_utils::Status
std_url_to_string(
    lyric_runtime::BytecodeInterpreter *,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *)
{
    auto *currentCoro = state->currentCoro();
    auto *heapManager = state->heapManager();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT (frame.numArguments() == 0);
    auto receiver = frame.getReceiver();
    UrlRef *instance;
    TU_ASSERT(receiver.castRef(instance));
    auto url = instance->getUrl();

    TU_RETURN_IF_NOT_OK (heapManager->loadStringOntoStack(url.toString()));

    return {};
}

tempo_utils::Status
std_url_parse_url(
    lyric_runtime::BytecodeInterpreter *,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *)
{
    auto *currentCoro = state->currentCoro();
    auto *heapManager = state->heapManager();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() >= 1);
    const auto &arg0 = frame.getArgument(0);
    lyric_runtime::StringRef *str;
    TU_ASSERT(arg0.getString(str));

    std::string urlString;
    if (!str->utf8Value(urlString))
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid url string");

    auto url = tempo_utils::Url::fromString(urlString);
    if (url.isValid()) {
        auto *segmentManager = state->segmentManager();

        auto *segment = currentCoro->peekSP();
        auto object = segment->getObject();
        auto symbol = object.findSymbol(lyric_common::SymbolPath({"Url"}));
        TU_ASSERT (symbol.isValid());

        lyric_runtime::InterpreterStatus status;
        auto descriptor = segmentManager->resolveDescriptor(segment,
            symbol.getLinkageSection(), symbol.getLinkageIndex(), status);
        lyric_runtime::DescriptorEntry *entry;
        TU_ASSERT (descriptor.getDescriptor(entry, lyric_object::LinkageSection::Struct));
        const auto *vtable = segmentManager->resolveStructVirtualTable(descriptor, status);
        TU_ASSERT(vtable != nullptr);

        // create a new url instance
        auto ref = heapManager->allocateRef<UrlRef>(vtable);
        currentCoro->pushData(ref);

        // set the url on the instance
        UrlRef *instance;
        TU_ASSERT (ref.castRef(instance));
        instance->setUrl(url);

    } else {
        TU_RETURN_IF_NOT_OK (heapManager->loadStatusOntoStack(
            tempo_utils::StatusCode::kInvalidArgument, "failed to parse url"));
    }

    return {};
}
