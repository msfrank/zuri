#ifndef ZURI_DISTRIBUTOR_DISTRIBUTION_LOADER_H
#define ZURI_DISTRIBUTOR_DISTRIBUTION_LOADER_H

#include <filesystem>

#include <lyric_runtime/abstract_loader.h>

namespace zuri_distributor {

    class DistributionLoader : public lyric_runtime::AbstractLoader {
    public:
        DistributionLoader(const std::filesystem::path &distributionRoot);

        tempo_utils::Result<bool> hasModule(
            const lyric_common::ModuleLocation &location) const override;
        tempo_utils::Result<Option<lyric_object::LyricObject>> loadModule(
            const lyric_common::ModuleLocation &location) override;
        tempo_utils::Result<Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>> loadPlugin(
            const lyric_common::ModuleLocation &location,
            const lyric_object::PluginSpecifier &specifier) override;

    private:
        std::filesystem::path m_distributionRoot;

        std::filesystem::path findModule(const lyric_common::ModuleLocation &location) const;
        std::filesystem::path moduleLocationToFilePath(
            const std::filesystem::path &packagesPath,
            const lyric_common::ModuleLocation &location) const;
        bool fileInfoIsValid(const std::filesystem::path &path) const;
    };
}

#endif // ZURI_DISTRIBUTOR_DISTRIBUTION_LOADER_H
