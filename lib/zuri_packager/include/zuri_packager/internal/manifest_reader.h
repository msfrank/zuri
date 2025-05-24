#ifndef ZURI_PACKAGER_INTERNAL_MANIFEST_READER_H
#define ZURI_PACKAGER_INTERNAL_MANIFEST_READER_H

#include <span>

#include <zuri_packager/generated/manifest.h>
#include <tempo_utils/integer_types.h>

namespace zuri_packager::internal {

    class ManifestReader {

    public:
        ManifestReader(std::span<const tu_uint8> bytes);

        bool isValid() const;

        zpk1::ManifestVersion getABI() const;

        const zpk1::NamespaceDescriptor *getNamespace(uint32_t index) const;
        uint32_t numNamespaces() const;

        const zpk1::AttrDescriptor *getAttr(uint32_t index) const;
        uint32_t numAttrs() const;

        const zpk1::EntryDescriptor *getEntry(uint32_t index) const;
        uint32_t numEntries() const;

        const zpk1::PathDescriptor *getPath(uint32_t index) const;
        const zpk1::PathDescriptor *findPath(std::string_view path) const;
        uint32_t numPaths() const;

        std::span<const tu_uint8> bytesView() const;

        std::string dumpJson() const;

    private:
        std::span<const tu_uint8> m_bytes;
        const zpk1::Manifest *m_manifest;
    };
}

#endif // ZURI_PACKAGER_INTERNAL_MANIFEST_READER_H
