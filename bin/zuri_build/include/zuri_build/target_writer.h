#ifndef ZURI_BUILD_TARGET_WRITER_H
#define ZURI_BUILD_TARGET_WRITER_H

#include <lyric_build/lyric_metadata.h>
#include <tempo_utils/tempdir_maker.h>
#include <zuri_packager/package_specifier.h>
#include <zuri_packager/package_dependency.h>
#include <zuri_packager/package_writer.h>

namespace zuri_build {

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
        void setProgramMain(const lyric_common::ModuleLocation &programMain);

        tempo_utils::Status addDependency(const zuri_packager::PackageSpecifier &specifier);

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
            lyric_common::ModuleLocation programMain;
            absl::flat_hash_map<zuri_packager::PackageId, zuri_packager::PackageVersion> dependencies;
        };
        std::unique_ptr<Priv> m_priv;

        tempo_utils::Status writePackageConfig();
    };
}

#endif // ZURI_BUILD_TARGET_WRITER_H