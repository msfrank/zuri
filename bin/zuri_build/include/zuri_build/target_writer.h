#ifndef LYRIC_BUILD_INTERNAL_PACKAGE_TASK_H
#define LYRIC_BUILD_INTERNAL_PACKAGE_TASK_H

#include <lyric_build/lyric_metadata.h>
#include <tempo_utils/tempdir_maker.h>
#include <zuri_packager/package_specifier.h>
#include <zuri_packager/package_writer.h>

#include "zuri_packager/package_dependency.h"

class TargetWriter {

public:
    TargetWriter(
        const std::filesystem::path &installRoot,
        const zuri_packager::PackageSpecifier &specifier);

    tempo_utils::Status configure();

    void setDescription(std::string_view description);
    void setOwner(std::string_view owner);
    void setHomepage(std::string_view homepage);
    void setLicense(std::string_view license);

    tempo_utils::Status addDependency(const zuri_packager::PackageDependency &dependency);

    tempo_utils::Status writeModule(
        const tempo_utils::UrlPath &modulePath,
        const lyric_build::LyricMetadata &metadata,
        std::shared_ptr<const tempo_utils::ImmutableBytes> content);

    tempo_utils::Result<std::filesystem::path> writeTarget();

private:
    std::filesystem::path m_installRoot;
    zuri_packager::PackageSpecifier m_specifier;

    struct Priv {
        std::unique_ptr<zuri_packager::PackageWriter> packageWriter;
        std::string description;
        std::string owner;
        std::string homepage;
        std::string license;
        absl::flat_hash_map<std::string, zuri_packager::PackageDependency> dependencies;
    };
    std::unique_ptr<Priv> m_priv;
};

#endif // LYRIC_BUILD_INTERNAL_PACKAGE_TASK_H