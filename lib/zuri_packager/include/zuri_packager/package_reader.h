#ifndef ZURI_PACKAGER_PACKAGE_READER_H
#define ZURI_PACKAGER_PACKAGE_READER_H

#include <filesystem>

#include <tempo_config/config_types.h>
#include <tempo_utils/integer_types.h>
#include <tempo_utils/memory_mapped_bytes.h>

#include "manifest_state.h"
#include "package_requirement.h"
#include "package_specifier.h"
#include "zuri_manifest.h"

namespace zuri_packager {

    class PackageReader {

    public:
        static tempo_utils::Result<std::shared_ptr<PackageReader>> create(
            std::shared_ptr<const tempo_utils::ImmutableBytes> packageBytes);
        static tempo_utils::Result<std::shared_ptr<PackageReader>> open(
            const std::filesystem::path &packagePath);

        bool isValid() const;

        tu_uint8 getVersion() const;
        tu_uint8 getFlags() const;
        ZuriManifest getManifest() const;

        tempo_utils::Result<PackageSpecifier> readPackageSpecifier() const;
        tempo_utils::Result<RequirementsMap> readRequirementsMap() const;
        tempo_utils::Result<tempo_config::ConfigMap> readPackageConfig() const;
        tempo_utils::Result<tempo_utils::Slice> readFileContents(
            const tempo_utils::UrlPath &entryPath,
            bool followSymlinks = false) const;

    private:
        tu_uint8 m_version;
        tu_uint8 m_flags;
        ZuriManifest m_manifest;
        tempo_utils::Slice m_contents;

        PackageReader(
            tu_uint8 version,
            tu_uint8 flags,
            ZuriManifest manifest,
            tempo_utils::Slice contents);
    };
}

#endif // ZURI_PACKAGER_PACKAGE_READER_H