
#include <zuri_packager/entry_walker.h>
#include <zuri_packager/generated/manifest.h>
#include <zuri_packager/internal/manifest_reader.h>

zuri_packager::EntryWalker::EntryWalker()
    : m_index(kInvalidOffsetU32)
{
}

zuri_packager::EntryWalker::EntryWalker(std::shared_ptr<const internal::ManifestReader> reader, tu_uint32 index)
    : m_reader(reader),
      m_index(index)
{
}

zuri_packager::EntryWalker::EntryWalker(const EntryWalker &other)
    : m_reader(other.m_reader),
      m_index(other.m_index)
{
}

bool
zuri_packager::EntryWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_index < m_reader->numEntries();
}

zuri_packager::EntryType
zuri_packager::EntryWalker::getEntryType() const
{
    auto *entry = m_reader->getEntry(m_index);
    if (entry == nullptr)
        return EntryType::Invalid;
    switch (entry->entry_type()) {
        case zpk1::EntryType::File:
            return EntryType::File;
        case zpk1::EntryType::Directory:
            return EntryType::Directory;
        case zpk1::EntryType::Link:
            return EntryType::Link;
        case zpk1::EntryType::Package:
            return EntryType::Package;
        default:
            return EntryType::Invalid;
    }
}

tempo_utils::UrlPath
zuri_packager::EntryWalker::getPath() const
{
    auto *entry = m_reader->getEntry(m_index);
    if (entry == nullptr)
        return {};
    if (entry->path() == nullptr)
        return {};
    return tempo_utils::UrlPath::fromString(entry->path()->c_str());
}

tu_uint64
zuri_packager::EntryWalker::getFileOffset() const
{
    auto *entry = m_reader->getEntry(m_index);
    if (entry == nullptr)
        return {};
    return entry->entry_offset();
}

tu_uint64
zuri_packager::EntryWalker::getFileSize() const
{
    auto *entry = m_reader->getEntry(m_index);
    if (entry == nullptr)
        return {};
    return entry->entry_size();
}

zuri_packager::EntryWalker
zuri_packager::EntryWalker::getLink() const
{
    auto *entry = m_reader->getEntry(m_index);
    if (entry == nullptr)
        return {};
    return EntryWalker(m_reader, entry->entry_link());
}

static zuri_packager::EntryWalker
resolve_link(const zuri_packager::EntryWalker &walker, int recursionDepth)
{
    if (recursionDepth == 0)
        return {};
    if (walker.getEntryType() != zuri_packager::EntryType::Link)
        return walker;
    return resolve_link(walker.getLink(), recursionDepth--);
}

zuri_packager::EntryWalker
zuri_packager::EntryWalker::resolveLink() const
{
    return resolve_link(*this, kMaximumLinkRecursion);
}

bool
zuri_packager::EntryWalker::hasAttr(const tempo_schema::AttrKey &key) const
{
    auto index = findIndexForAttr(key);
    return index != kInvalidOffsetU32;
}

bool
zuri_packager::EntryWalker::hasAttr(const tempo_schema::AttrValidator &validator) const
{
    return hasAttr(validator.getKey());
}

tu_uint32
zuri_packager::EntryWalker::findIndexForAttr(const tempo_schema::AttrKey &key) const
{
    if (!isValid())
        return kInvalidOffsetU32;
    auto *entry = m_reader->getEntry(m_index);
    TU_ASSERT (entry != nullptr);
    auto *attrs = entry->entry_attrs();
    if (attrs == nullptr)    // entry has no attrs
        return kInvalidOffsetU32;
    for (const auto attrIndex : *attrs) {
        auto *attr = m_reader->getAttr(attrIndex);
        TU_ASSERT (attr != nullptr);
        auto *ns = m_reader->getNamespace(attr->attr_ns());
        TU_ASSERT (ns != nullptr);
        auto *nsUrl = ns->ns_url();
        if (nsUrl == nullptr)
            continue;
        if (std::string_view(key.ns) == nsUrl->string_view() && key.id == attr->attr_id())
            return attrIndex;
    }
    return kInvalidOffsetU32;
}

int
zuri_packager::EntryWalker::numAttrs() const
{
    if (!isValid())
        return 0;
    auto *entry = m_reader->getEntry(m_index);
    TU_ASSERT (entry != nullptr);
    auto *attrs = entry->entry_attrs();
    if (attrs == nullptr)    // entry has no attrs
        return 0;
    return attrs->size();
}

zuri_packager::EntryWalker
zuri_packager::EntryWalker::getChild(tu_uint32 index) const
{
    if (!isValid())
        return {};
    auto *entry = m_reader->getEntry(m_index);
    TU_ASSERT (entry != nullptr);
    auto *children = entry->entry_children();
    if (children == nullptr)    // span has no children
        return {};
    if (children->size() <= index)
        return {};
    return EntryWalker(m_reader, children->Get(index));
}

zuri_packager::EntryWalker
zuri_packager::EntryWalker::getChild(std::string_view name) const
{
    if (!isValid())
        return {};
    auto *entry = m_reader->getEntry(m_index);
    TU_ASSERT (entry != nullptr);
    auto *children = entry->entry_children();
    if (children == nullptr)    // span has no children
        return {};
    for (tu_uint32 i = 0; i < children->size(); i++) {
        EntryWalker child(m_reader, children->Get(i));
        auto path = child.getPath();
        if (name == path.getLast().partView())
            return child;
    }
    return {};
}

int
zuri_packager::EntryWalker::numChildren() const
{
    if (!isValid())
        return 0;
    auto *entry = m_reader->getEntry(m_index);
    TU_ASSERT (entry != nullptr);
    auto *children = entry->entry_children();
    if (children == nullptr)    // span has no children
        return 0;
    return children->size();
}
