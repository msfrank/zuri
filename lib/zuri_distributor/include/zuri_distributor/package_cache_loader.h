#ifndef ZURI_DISTRIBUTOR_PACKAGE_CACHE_LOADER_H
#define ZURI_DISTRIBUTOR_PACKAGE_CACHE_LOADER_H

#include <filesystem>

#include <lyric_runtime/abstract_loader.h>

#include "abstract_package_cache.h"

namespace zuri_distributor {

    class PackageCacheLoader : public lyric_runtime::AbstractLoader {
    public:
        explicit PackageCacheLoader(std::shared_ptr<AbstractPackageCache> readonlyPackageCache);

        std::shared_ptr<AbstractPackageCache> getPackageCache() const;

        tempo_utils::Result<bool> hasModule(
            const lyric_common::ModuleLocation &location) const override;
        tempo_utils::Result<Option<lyric_object::LyricObject>> loadModule(
            const lyric_common::ModuleLocation &location) override;
        tempo_utils::Result<bool> hasPlugin(
            const lyric_common::ModuleLocation &location,
            const lyric_object::PluginSpecifier &specifier) const override;
        tempo_utils::Result<Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>> loadPlugin(
            const lyric_common::ModuleLocation &location,
            const lyric_object::PluginSpecifier &specifier) override;
        tempo_utils::Result<bool> hasResource(
            const lyric_common::ModuleLocation &location) const override;
        tempo_utils::Result<Option<std::shared_ptr<const tempo_utils::ImmutableBytes>>> loadResource(
            const lyric_common::ModuleLocation &location) override;

    private:
        std::shared_ptr<AbstractPackageCache> m_readonlyPackageCache;

        tempo_utils::Result<std::filesystem::path> findModule(
            const lyric_common::ModuleLocation &location,
            std::string_view dotSuffix) const;
        tempo_utils::Result<std::filesystem::path> findResource(
            const lyric_common::ModuleLocation &location) const;
    };
}

#endif // ZURI_DISTRIBUTOR_PACKAGE_CACHE_LOADER_H
