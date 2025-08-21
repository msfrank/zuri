#ifndef ZURI_PACKAGER_PACKAGE_READER_LOADER_H
#define ZURI_PACKAGER_PACKAGE_READER_LOADER_H

#include <filesystem>

#include <lyric_runtime/abstract_loader.h>

#include "package_reader.h"

namespace zuri_packager {

    class PackageReaderLoader : public lyric_runtime::AbstractLoader {
    public:
        static tempo_utils::Result<std::shared_ptr<PackageReaderLoader>> create(
            std::shared_ptr<PackageReader> reader,
            const std::filesystem::path &tempRoot);

        tempo_utils::Result<bool> hasModule(
            const lyric_common::ModuleLocation &location) const override;
        tempo_utils::Result<Option<lyric_object::LyricObject>> loadModule(
            const lyric_common::ModuleLocation &location) override;
        tempo_utils::Result<Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>> loadPlugin(
            const lyric_common::ModuleLocation &location,
            const lyric_object::PluginSpecifier &specifier) override;

    private:
        std::shared_ptr<PackageReader> m_reader;
        PackageSpecifier m_specifier;
        std::filesystem::path m_packageDirectory;
        std::filesystem::path m_tempDirectory;

        PackageReaderLoader(
            std::shared_ptr<PackageReader> reader,
            const PackageSpecifier &specifier,
            const std::filesystem::path &packageDirectory,
            const std::filesystem::path &tempDirectory);

        tempo_utils::Result<std::filesystem::path> findModule(
            const lyric_common::ModuleLocation &location,
            std::string_view dotSuffix) const;
    };
}

#endif // ZURI_PACKAGER_PACKAGE_READER_LOADER_H