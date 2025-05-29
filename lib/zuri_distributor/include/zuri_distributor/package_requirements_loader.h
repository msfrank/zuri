#ifndef ZURI_DISTRIBUTOR_PACKAGE_REQUIREMENTS_LOADER_H
#define ZURI_DISTRIBUTOR_PACKAGE_REQUIREMENTS_LOADER_H

#include <lyric_runtime/abstract_loader.h>

namespace zuri_distributor {
    class PackageRequirementsLoader : public lyric_runtime::AbstractLoader {
    public:
        explicit PackageRequirementsLoader(
            const absl::flat_hash_map<std::string, std::filesystem::path> &importPackages);

        tempo_utils::Result<bool> hasModule(const lyric_common::ModuleLocation &location) const override;

        tempo_utils::Result<Option<lyric_object::LyricObject>> loadModule(
            const lyric_common::ModuleLocation &location) override;

        tempo_utils::Result<Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>> loadPlugin(
            const lyric_common::ModuleLocation &location,
            const lyric_object::PluginSpecifier &specifier) override;

    private:
        absl::flat_hash_map<std::string, std::filesystem::path> m_importPackages;

        tempo_utils::Result<std::filesystem::path> findModule(
            const lyric_common::ModuleLocation &location,
            std::string_view dotSuffix) const;
    };
}

#endif // ZURI_DISTRIBUTOR_PACKAGE_REQUIREMENTS_LOADER_H
