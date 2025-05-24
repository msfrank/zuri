
#include <absl/container/flat_hash_map.h>
#include <unicode/umachine.h>
#include <unicode/ustring.h>

#include <zuri_packager/generated/manifest.h>
#include <zuri_packager/manifest_attr.h>
#include <zuri_packager/manifest_entry.h>
#include <zuri_packager/manifest_namespace.h>
#include <zuri_packager/manifest_state.h>
#include <zuri_packager/package_types.h>
#include <tempo_utils/memory_bytes.h>

zuri_packager::ManifestState::ManifestState()
{
}

bool
zuri_packager::ManifestState::hasNamespace(const tempo_utils::Url &nsUrl) const
{
    return getNamespace(nsUrl) != nullptr;
}

zuri_packager::ManifestNamespace *
zuri_packager::ManifestState::getNamespace(int index) const
{
    if (0 <= index && std::cmp_less(index, m_manifestNamespaces.size()))
        return m_manifestNamespaces.at(index);
    return nullptr;
}

zuri_packager::ManifestNamespace *
zuri_packager::ManifestState::getNamespace(const tempo_utils::Url &nsUrl) const
{
    if (!m_namespaceIndex.contains(nsUrl))
        return nullptr;
    return getNamespace(m_namespaceIndex.at(nsUrl));
}

tempo_utils::Result<zuri_packager::ManifestNamespace *>
zuri_packager::ManifestState::putNamespace(const tempo_utils::Url &nsUrl)
{
    if (m_namespaceIndex.contains(nsUrl)) {
        auto index = m_namespaceIndex.at(nsUrl);
        TU_ASSERT (0 <= index && index < m_manifestNamespaces.size());
        return m_manifestNamespaces.at(index);
    }
    NamespaceAddress address(m_manifestNamespaces.size());
    auto *ns = new ManifestNamespace(nsUrl, address, this);
    m_manifestNamespaces.push_back(ns);
    m_namespaceIndex[nsUrl] = address.getAddress();
    return ns;
}

std::vector<zuri_packager::ManifestNamespace *>::const_iterator
zuri_packager::ManifestState::namespacesBegin() const
{
    return m_manifestNamespaces.cbegin();
}

std::vector<zuri_packager::ManifestNamespace *>::const_iterator
zuri_packager::ManifestState::namespacesEnd() const
{
    return m_manifestNamespaces.cend();
}

int
zuri_packager::ManifestState::numNamespaces() const
{
    return m_manifestNamespaces.size();
}

tempo_utils::Result<zuri_packager::ManifestAttr *>
zuri_packager::ManifestState::appendAttr(AttrId id, const tempo_schema::AttrValue &value)
{
    AttrAddress address(m_manifestAttrs.size());
    auto *attr = new ManifestAttr(id, value, address, this);
    m_manifestAttrs.push_back(attr);
    return attr;
}

zuri_packager::ManifestAttr *
zuri_packager::ManifestState::getAttr(int index) const
{
    if (0 <= index && std::cmp_less(index, m_manifestAttrs.size()))
        return m_manifestAttrs.at(index);
    return nullptr;
}

std::vector<zuri_packager::ManifestAttr *>::const_iterator
zuri_packager::ManifestState::attrsBegin() const
{
    return m_manifestAttrs.cbegin();
}

std::vector<zuri_packager::ManifestAttr *>::const_iterator
zuri_packager::ManifestState::attrsEnd() const
{
    return m_manifestAttrs.cend();
}

int
zuri_packager::ManifestState::numAttrs() const
{
    return m_manifestAttrs.size();
}

bool
zuri_packager::ManifestState::hasEntry(const tempo_utils::UrlPath &path) const
{
    return getEntry(path) != nullptr;
}

zuri_packager::ManifestEntry *
zuri_packager::ManifestState::getEntry(int index) const
{
    if (0 <= index && std::cmp_less(index, m_manifestEntries.size()))
        return m_manifestEntries.at(index);
    return nullptr;
}

zuri_packager::ManifestEntry *
zuri_packager::ManifestState::getEntry(const tempo_utils::UrlPath &path) const
{
    if (!m_pathIndex.contains(path))
        return nullptr;
    return getEntry(m_pathIndex.at(path));
}

tempo_utils::Result<zuri_packager::ManifestEntry *>
zuri_packager::ManifestState::appendEntry(EntryType type, const tempo_utils::UrlPath &path)
{
    if (m_pathIndex.contains(path))
        return PackageStatus::forCondition(
            PackageCondition::kDuplicateEntry, "entry already exists at path");

    if (path.isEmpty()) {
        // an empty path indicates the package (i.e. root) entry
        EntryAddress address(m_manifestEntries.size());
        auto *entry = new ManifestEntry(type, path, address, this);
        m_manifestEntries.push_back(entry);
        m_pathIndex[path] = address.getAddress();
        return entry;
    } else {
        // a non-empty path must have a parent
        auto parentPath = path.getInit();
        if (!m_pathIndex.contains(parentPath))
            return PackageStatus::forCondition(
                PackageCondition::kPackageInvariant, "entry is missing parent");
        auto *parent = getEntry(m_pathIndex.at(parentPath));

        EntryAddress address(m_manifestEntries.size());
        auto *entry = new ManifestEntry(type, path, address, this);
        m_manifestEntries.push_back(entry);
        m_pathIndex[path] = address.getAddress();
        parent->putChild(entry);

        return entry;
    }
}

std::vector<zuri_packager::ManifestEntry *>::const_iterator
zuri_packager::ManifestState::entriesBegin() const
{
    return m_manifestEntries.cbegin();
}

std::vector<zuri_packager::ManifestEntry *>::const_iterator
zuri_packager::ManifestState::entriesEnd() const
{
    return m_manifestEntries.cend();
}

int
zuri_packager::ManifestState::numEntries() const
{
    return m_manifestEntries.size();
}

static std::pair<zpk1::Value,flatbuffers::Offset<void>>
serialize_value(flatbuffers::FlatBufferBuilder &buffer, const tempo_schema::AttrValue &value)
{
    switch (value.getType()) {
        case tempo_schema::ValueType::Nil: {
            auto type = zpk1::Value::TrueFalseNilValue;
            auto offset = zpk1::CreateTrueFalseNilValue(buffer, zpk1::TrueFalseNil::Nil).Union();
            return {type, offset};
        }
        case tempo_schema::ValueType::Bool: {
            auto type = zpk1::Value::TrueFalseNilValue;
            auto tfn = value.getBool()? zpk1::TrueFalseNil::True : zpk1::TrueFalseNil::False;
            auto offset = zpk1::CreateTrueFalseNilValue(buffer, tfn).Union();
            return {type, offset};
        }
        case tempo_schema::ValueType::Int64: {
            auto type = zpk1::Value::Int64Value;
            auto offset = zpk1::CreateInt64Value(buffer, value.getInt64()).Union();
            return {type, offset};
        }
        case tempo_schema::ValueType::Float64: {
            auto type = zpk1::Value::Float64Value;
            auto offset = zpk1::CreateFloat64Value(buffer, value.getFloat64()).Union();
            return {type, offset};
        }
        case tempo_schema::ValueType::UInt64: {
            auto type = zpk1::Value::UInt64Value;
            auto offset = zpk1::CreateUInt64Value(buffer, value.getUInt64()).Union();
            return {type, offset};
        }
        case tempo_schema::ValueType::UInt32: {
            auto type = zpk1::Value::UInt32Value;
            auto offset = zpk1::CreateUInt32Value(buffer, value.getUInt32()).Union();
            return {type, offset};
        }
        case tempo_schema::ValueType::UInt16: {
            auto type = zpk1::Value::UInt16Value;
            auto offset = zpk1::CreateUInt16Value(buffer, value.getUInt16()).Union();
            return {type, offset};
        }
        case tempo_schema::ValueType::UInt8: {
            auto type = zpk1::Value::UInt8Value;
            auto offset = zpk1::CreateUInt8Value(buffer, value.getUInt8()).Union();
            return {type, offset};
        }
        case tempo_schema::ValueType::String: {
            auto type = zpk1::Value::StringValue;
            auto offset = zpk1::CreateStringValue(buffer, buffer.CreateSharedString(value.stringView())).Union();
            return {type, offset};
        }
        default:
            TU_UNREACHABLE();
    }
}

tempo_utils::Result<zuri_packager::ZuriManifest>
zuri_packager::ManifestState::toManifest() const
{
    flatbuffers::FlatBufferBuilder buffer;

    std::vector<flatbuffers::Offset<zpk1::NamespaceDescriptor>> namespaces_vector;
    std::vector<flatbuffers::Offset<zpk1::AttrDescriptor>> attrs_vector;
    std::vector<flatbuffers::Offset<zpk1::EntryDescriptor>> entries_vector;
    std::vector<flatbuffers::Offset<zpk1::PathDescriptor>> paths_vector;

    // serialize namespaces
    for (const auto *ns : m_manifestNamespaces) {
        auto fb_nsUrl = buffer.CreateString(ns->getNsUrl().toString());
        namespaces_vector.push_back(zpk1::CreateNamespaceDescriptor(buffer, fb_nsUrl));
    }
    auto fb_namespaces = buffer.CreateVector(namespaces_vector);

    // serialize attributes
    for (const auto *attr : m_manifestAttrs) {
        auto id = attr->getAttrId();
        auto value = attr->getAttrValue();
        auto p = serialize_value(buffer, value);
        attrs_vector.push_back(zpk1::CreateAttrDescriptor(buffer,
            id.getAddress().getAddress(), id.getType(), p.first, p.second));
    }
    auto fb_attrs = buffer.CreateVector(attrs_vector);

    // serialize entries
    for (const auto *entry : m_manifestEntries) {
        auto pathString = entry->getEntryPath().toString();
        auto fb_path = buffer.CreateSharedString(pathString);

        // map the entry type
        zpk1::EntryType type;
        switch (entry->getEntryType()) {
            case EntryType::Package:
                type = zpk1::EntryType::Package;
                break;
            case EntryType::File:
                type = zpk1::EntryType::File;
                break;
            case EntryType::Directory:
                type = zpk1::EntryType::Directory;
                break;
            case EntryType::Link:
                type = zpk1::EntryType::Link;
                break;
            default:
                return PackageStatus::forCondition(
                    PackageCondition::kPackageInvariant, "invalid entry type");
        }

        // serialize entry attrs
        std::vector<uint32_t> entry_attrs;
        for (auto iterator = entry->attrsBegin(); iterator != entry->attrsEnd(); iterator++) {
            entry_attrs.push_back(iterator->second.getAddress());
        }
        auto fb_entry_attrs = buffer.CreateVector(entry_attrs);

        // serialize entry children
        std::vector<uint32_t> entry_children;
        for (auto iterator = entry->childrenBegin(); iterator != entry->childrenEnd(); iterator++) {
            entry_children.push_back(iterator->second.getAddress());
        }
        auto fb_entry_children = buffer.CreateVector(entry_children);

        // append entry
        entries_vector.push_back(zpk1::CreateEntryDescriptor(buffer,
            fb_path, type, fb_entry_attrs, fb_entry_children,
            entry->getEntryOffset(), entry->getEntrySize(),
            entry->getEntryDict().getAddress(), entry->getEntryLink().getAddress()));

        // append path
        paths_vector.push_back(zpk1::CreatePathDescriptor(buffer, fb_path, entry->getAddress().getAddress()));
    }
    auto fb_entries = buffer.CreateVector(entries_vector);

    // serialize sorted paths
    auto fb_paths = buffer.CreateVectorOfSortedTables(&paths_vector);

    // build package from buffer
    zpk1::ManifestBuilder manifestBuilder(buffer);

    manifestBuilder.add_abi(zpk1::ManifestVersion::Version1);
    manifestBuilder.add_namespaces(fb_namespaces);
    manifestBuilder.add_attrs(fb_attrs);
    manifestBuilder.add_entries(fb_entries);
    manifestBuilder.add_paths(fb_paths);

    // serialize package and mark the buffer as finished
    auto manifest = manifestBuilder.Finish();
    buffer.Finish(manifest, zpk1::ManifestIdentifier());

    // copy the flatbuffer into our own byte array and instantiate package
    auto bytes = tempo_utils::MemoryBytes::copy(buffer.GetBufferSpan());
    return ZuriManifest(bytes);
}
