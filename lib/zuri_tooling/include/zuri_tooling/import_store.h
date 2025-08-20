#ifndef ZURI_TOOLING_IMPORT_STORE_H
#define ZURI_TOOLING_IMPORT_STORE_H

#include <lyric_build/build_types.h>
#include <tempo_utils/status.h>
#include <zuri_packager/package_specifier.h>

namespace zuri_tooling {

    struct ImportEntry {
        zuri_packager::PackageVersion version;
    };

    class ImportStore {
    public:
        explicit ImportStore(const tempo_config::ConfigMap &importsMap);

        tempo_utils::Status configure();

        bool hasImport(const zuri_packager::PackageId &packageId) const;
        std::shared_ptr<const ImportEntry> getImport(const zuri_packager::PackageId &packageId) const;
        absl::flat_hash_map<zuri_packager::PackageId,std::shared_ptr<const ImportEntry>>::const_iterator importsBegin() const;
        absl::flat_hash_map<zuri_packager::PackageId,std::shared_ptr<const ImportEntry>>::const_iterator importsEnd() const;
        int numImports() const;

    private:
        tempo_config::ConfigMap m_importsMap;

        absl::flat_hash_map<zuri_packager::PackageId,std::shared_ptr<const ImportEntry>> m_importEntries;
    };
}

#endif // ZURI_TOOLING_IMPORT_STORE_H
