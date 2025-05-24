
#include <flatbuffers/idl.h>

#include <zuri_packager/generated/manifest_schema.h>
#include <zuri_packager/internal/manifest_reader.h>
#include <tempo_utils/log_stream.h>

zuri_packager::internal::ManifestReader::ManifestReader(std::span<const tu_uint8> bytes)
    : m_bytes(bytes)
{
    m_manifest = zpk1::GetManifest(m_bytes.data());
}

bool
zuri_packager::internal::ManifestReader::isValid() const
{
    return m_manifest != nullptr;
}

zpk1::ManifestVersion
zuri_packager::internal::ManifestReader::getABI() const
{
    if (m_manifest == nullptr)
        return zpk1::ManifestVersion::Unknown;
    return m_manifest->abi();
}

const zpk1::NamespaceDescriptor *
zuri_packager::internal::ManifestReader::getNamespace(uint32_t index) const
{
    if (m_manifest == nullptr)
        return nullptr;
    if (m_manifest->namespaces() && index < m_manifest->namespaces()->size())
        return m_manifest->namespaces()->Get(index);
    return nullptr;
}

uint32_t
zuri_packager::internal::ManifestReader::numNamespaces() const
{
    if (m_manifest == nullptr)
        return 0;
    return m_manifest->namespaces()? m_manifest->namespaces()->size() : 0;
}

const zpk1::EntryDescriptor *
zuri_packager::internal::ManifestReader::getEntry(uint32_t index) const
{
    if (m_manifest == nullptr)
        return nullptr;
    if (m_manifest->entries() && index < m_manifest->entries()->size())
        return m_manifest->entries()->Get(index);
    return nullptr;
}

uint32_t
zuri_packager::internal::ManifestReader::numEntries() const
{
    if (m_manifest == nullptr)
        return 0;
    return m_manifest->entries()? m_manifest->entries()->size() : 0;
}

const zpk1::AttrDescriptor *
zuri_packager::internal::ManifestReader::getAttr(uint32_t index) const
{
    if (m_manifest == nullptr)
        return nullptr;
    if (m_manifest->attrs() && index < m_manifest->attrs()->size())
        return m_manifest->attrs()->Get(index);
    return nullptr;
}

uint32_t
zuri_packager::internal::ManifestReader::numAttrs() const
{
    if (m_manifest == nullptr)
        return 0;
    return m_manifest->attrs()? m_manifest->attrs()->size() : 0;
}

const zpk1::PathDescriptor *
zuri_packager::internal::ManifestReader::getPath(uint32_t index) const
{
    if (m_manifest == nullptr)
        return nullptr;
    if (m_manifest->paths() && index < m_manifest->paths()->size())
        return m_manifest->paths()->Get(index);
    return nullptr;
}

const zpk1::PathDescriptor *
zuri_packager::internal::ManifestReader::findPath(std::string_view path) const
{
    if (m_manifest == nullptr)
        return nullptr;
    std::string fullPath(path);
    return m_manifest->paths() ? m_manifest->paths()->LookupByKey(fullPath.c_str()) : nullptr;
}

uint32_t
zuri_packager::internal::ManifestReader::numPaths() const
{
    if (m_manifest == nullptr)
        return 0;
    return m_manifest->paths()? m_manifest->paths()->size() : 0;
}

std::span<const tu_uint8>
zuri_packager::internal::ManifestReader::bytesView() const
{
    return m_bytes;
}

std::string
zuri_packager::internal::ManifestReader::dumpJson() const
{
    flatbuffers::Parser parser;
    parser.Parse((const char *) zuri_packager::schema::manifest::data);
    parser.opts.strict_json = true;

    std::string jsonData;
    auto *err = GenText(parser, m_bytes.data(), &jsonData);
    TU_ASSERT (err == nullptr);
    return jsonData;
}
