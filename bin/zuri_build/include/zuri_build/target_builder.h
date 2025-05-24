#ifndef ZURI_BUILD_TARGET_BUILDER_H
#define ZURI_BUILD_TARGET_BUILDER_H

#include <lyric_build/lyric_builder.h>

#include "build_graph.h"

class TargetBuilder {
public:
    TargetBuilder(
        std::shared_ptr<BuildGraph> buildGraph,
        lyric_build::LyricBuilder *builder,
        const std::filesystem::path &installRoot);

    tempo_utils::Result<std::filesystem::path> buildTarget(const std::string &targetName);

private:
    std::shared_ptr<BuildGraph> m_buildGraph;
    lyric_build::LyricBuilder *m_builder;
    std::filesystem::path m_installRoot;

    tempo_utils::Result<std::filesystem::path> buildProgramTarget(
        const std::string &targetName,
        const TargetEntry &programTarget);
    tempo_utils::Result<std::filesystem::path> buildLibraryTarget(
        const std::string &targetName,
        const TargetEntry &libraryTarget);
};

#endif // ZURI_BUILD_TARGET_BUILDER_H
