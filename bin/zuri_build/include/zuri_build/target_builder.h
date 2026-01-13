#ifndef ZURI_BUILD_TARGET_BUILDER_H
#define ZURI_BUILD_TARGET_BUILDER_H

#include <lyric_build/lyric_builder.h>
#include <tempo_utils/result.h>
#include <zuri_distributor/runtime.h>
#include <zuri_tooling/build_graph.h>
#include <zuri_tooling/package_manager.h>

namespace zuri_build {

    class TargetBuilder {
    public:
        TargetBuilder(
            std::shared_ptr<zuri_distributor::Runtime> runtime,
            std::shared_ptr<zuri_tooling::BuildGraph> buildGraph,
            lyric_build::LyricBuilder *builder,
            absl::flat_hash_map<std::string,tempo_utils::Url> &&targetBases,
            const std::filesystem::path &installRoot);

        tempo_utils::Result<std::filesystem::path> buildTarget(const std::string &targetName);

    private:
        std::shared_ptr<zuri_distributor::Runtime> m_runtime;
        std::shared_ptr<zuri_tooling::BuildGraph> m_buildGraph;
        lyric_build::LyricBuilder *m_builder;
        absl::flat_hash_map<std::string,tempo_utils::Url> m_targetBases;
        std::filesystem::path m_installRoot;

        tempo_utils::Result<std::filesystem::path> buildProgramTarget(
            const std::string &targetName,
            std::shared_ptr<const zuri_tooling::TargetEntry> targetEntry,
            std::shared_ptr<lyric_importer::ShortcutResolver> targetShortcuts);
        tempo_utils::Result<std::filesystem::path> buildLibraryTarget(
            const std::string &targetName,
            std::shared_ptr<const zuri_tooling::TargetEntry> targetEntry,
            std::shared_ptr<lyric_importer::ShortcutResolver> targetShortcuts);
    };
}

#endif // ZURI_BUILD_TARGET_BUILDER_H
