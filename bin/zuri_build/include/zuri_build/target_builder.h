#ifndef ZURI_BUILD_TARGET_BUILDER_H
#define ZURI_BUILD_TARGET_BUILDER_H

#include <lyric_build/lyric_builder.h>
#include <zuri_distributor/distributor_result.h>
#include <zuri_distributor/package_cache.h>

#include "build_graph.h"

class TargetBuilder {
public:
    TargetBuilder(
        std::shared_ptr<BuildGraph> buildGraph,
        lyric_build::LyricBuilder *builder,
        std::shared_ptr<lyric_importer::ShortcutResolver> shortcutResolver,
        std::shared_ptr<zuri_distributor::PackageCache> targetPackageCache,
        const std::filesystem::path &installRoot);

    tempo_utils::Result<std::filesystem::path> buildTarget(
        const std::string &targetName,
        const absl::flat_hash_map<std::string,std::string> &targetShortcuts);

private:
    std::shared_ptr<BuildGraph> m_buildGraph;
    lyric_build::LyricBuilder *m_builder;
    std::shared_ptr<lyric_importer::ShortcutResolver> m_shortcutResolver;
    std::shared_ptr<zuri_distributor::PackageCache> m_targetPackageCache;
    std::filesystem::path m_installRoot;

    tempo_utils::Result<std::filesystem::path> buildProgramTarget(
        const std::string &targetName,
        std::shared_ptr<const zuri_tooling::TargetEntry> programTarget);
    tempo_utils::Result<std::filesystem::path> buildLibraryTarget(
        const std::string &targetName,
        std::shared_ptr<const zuri_tooling::TargetEntry> libraryTarget);
};

#endif // ZURI_BUILD_TARGET_BUILDER_H
