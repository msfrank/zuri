#ifndef LYRIC_BUILD_INTERNAL_PACKAGE_TASK_H
#define LYRIC_BUILD_INTERNAL_PACKAGE_TASK_H

#include <lyric_build/lyric_metadata.h>
#include <tempo_utils/tempdir_maker.h>
#include <zuri_packager/package_specifier.h>

class TargetWriter {

public:
    TargetWriter(
        const std::filesystem::path &installRoot,
        const zuri_packager::PackageSpecifier &specifier);

    tempo_utils::Status configure();

    void setDescription(std::string_view description);
    void setOwner(std::string_view owner);
    void setLicense(std::string_view license);

    tempo_utils::Status addDependency(const zuri_packager::PackageSpecifier &specifier);

    tempo_utils::Status writeModule(
        const tempo_utils::UrlPath &filePath,
        const lyric_build::LyricMetadata &metadata,
        std::shared_ptr<const tempo_utils::ImmutableBytes> content);

private:
    std::filesystem::path m_installRoot;
    zuri_packager::PackageSpecifier m_specifier;
    std::string m_description;
    std::string m_owner;
    std::string m_license;

    std::unique_ptr<tempo_utils::TempdirMaker> m_packagePath;
};

#endif // LYRIC_BUILD_INTERNAL_PACKAGE_TASK_H