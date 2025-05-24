#ifndef ZURI_PACKAGER_MANIFEST_STATE_H
#define ZURI_PACKAGER_MANIFEST_STATE_H

#include <tempo_utils/url.h>

#include "package_types.h"
#include "zuri_manifest.h"

namespace zuri_packager {

    // forward declarations
    class ManifestNamespace;
    class ManifestEntry;
    class ManifestAttr;

    class ManifestState {

    public:
        ManifestState();

        bool hasNamespace(const tempo_utils::Url &nsUrl) const;
        ManifestNamespace *getNamespace(int index) const;
        ManifestNamespace *getNamespace(const tempo_utils::Url &nsUrl) const;
        tempo_utils::Result<ManifestNamespace *> putNamespace(const tempo_utils::Url &nsUrl);
        std::vector<ManifestNamespace *>::const_iterator namespacesBegin() const;
        std::vector<ManifestNamespace *>::const_iterator namespacesEnd() const;
        int numNamespaces() const;

        tempo_utils::Result<ManifestAttr *> appendAttr(AttrId id, const tempo_schema::AttrValue &value);
        ManifestAttr *getAttr(int index) const;
        std::vector<ManifestAttr *>::const_iterator attrsBegin() const;
        std::vector<ManifestAttr *>::const_iterator attrsEnd() const;
        int numAttrs() const;

        bool hasEntry(const tempo_utils::UrlPath &path) const;
        ManifestEntry *getEntry(int index) const;
        ManifestEntry *getEntry(const tempo_utils::UrlPath &path) const;
        tempo_utils::Result<ManifestEntry *> appendEntry(EntryType type, const tempo_utils::UrlPath &path);
        std::vector<ManifestEntry *>::const_iterator entriesBegin() const;
        std::vector<ManifestEntry *>::const_iterator entriesEnd() const;
        int numEntries() const;

        tempo_utils::Result<ZuriManifest> toManifest() const;

    private:
        std::vector<ManifestNamespace *> m_manifestNamespaces;
        std::vector<ManifestEntry *> m_manifestEntries;
        std::vector<ManifestAttr *> m_manifestAttrs;
        absl::flat_hash_map<tempo_utils::Url,tu_uint32> m_namespaceIndex;
        absl::flat_hash_map<tempo_utils::UrlPath,tu_uint32> m_pathIndex;
    };
}

#endif // ZURI_PACKAGER_MANIFEST_STATE_H
