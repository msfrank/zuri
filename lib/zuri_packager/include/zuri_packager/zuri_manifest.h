#ifndef ZURI_PACKAGER_LYRIC_MANIFEST_H
#define ZURI_PACKAGER_LYRIC_MANIFEST_H

#include <span>

#include <tempo_utils/immutable_bytes.h>

#include "entry_walker.h"
#include "package_types.h"

namespace zuri_packager {

    class ZuriManifest {

    public:
        ZuriManifest();
        ZuriManifest(std::shared_ptr<const tempo_utils::ImmutableBytes> immutableBytes);
        ZuriManifest(std::span<const tu_uint8> unownedBytes);
        ZuriManifest(const ZuriManifest &other);

        bool isValid() const;

        ManifestVersion getABI() const;

        EntryWalker getRoot() const;

        bool hasEntry(const tempo_utils::UrlPath &entryPath) const;
        EntryWalker getEntry(tu_uint32 index) const;
        EntryWalker getEntry(const tempo_utils::UrlPath &entryPath) const;
        tu_uint32 numEntries() const;

        std::shared_ptr<const internal::ManifestReader> getReader() const;
        std::span<const tu_uint8> bytesView() const;

        std::string dumpJson() const;

        static bool verify(std::span<const tu_uint8> bytes);

    private:
        std::shared_ptr<const tempo_utils::ImmutableBytes> m_bytes;
        std::shared_ptr<const internal::ManifestReader> m_reader;
    };
}

#endif // ZURI_PACKAGER_LYRIC_MANIFEST_H
