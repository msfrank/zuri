#ifndef ZURI_BUILD_COLLECT_MODULES_TASK_H
#define ZURI_BUILD_COLLECT_MODULES_TASK_H

#include <absl/container/flat_hash_set.h>

#include <lyric_build/base_task.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_compiler/lyric_compiler.h>
#include <lyric_parser/lyric_parser.h>

namespace zuri_build {

    class CollectModulesTask : public lyric_build::BaseTask {

        enum class Phase {
            Initial,
            CollectModules,
            Complete,
        };

        enum class CollectType {
            Object,
            Plugin,
        };

    public:
        CollectModulesTask(
            const lyric_build::BuildGeneration &generation,
            const lyric_build::TaskKey &key,
            std::weak_ptr<lyric_build::BuildState> buildState,
            std::shared_ptr<tempo_tracing::TraceSpan> span);

        tempo_utils::Status configureTask(const lyric_build::TaskSettings &taskSettings) override;
        tempo_utils::Status deduplicateTask(lyric_build::TaskHash &taskHash) override;
        tempo_utils::Status runTask(lyric_build::TempDirectory *tempDirectory) override;

    private:
        Phase m_phase;
        absl::flat_hash_map<lyric_build::TaskKey,CollectType> m_collectTargets;
        absl::flat_hash_map<tempo_utils::UrlPath,lyric_build::TaskKey> m_artifactsToLink;
        absl::flat_hash_set<tempo_utils::UrlPath> m_modulesFound;

        tempo_utils::Status initial(const lyric_build::TaskSettings &settings);
        tempo_utils::Status collectModules(const lyric_build::TaskSettings &settings);
        tempo_utils::Status collectObject(
            const tempo_utils::UrlPath &objectArtifactPath,
            const lyric_build::TaskKey &taskKey,
            std::shared_ptr<lyric_build::AbstractArtifactCache> &artifactCache);
        tempo_utils::Status collectPlugin(
            const tempo_utils::UrlPath &pluginArtifactPath,
            const lyric_build::TaskKey &taskKey,
            std::shared_ptr<lyric_build::AbstractArtifactCache> &artifactCache);
    };

    lyric_build::BaseTask *new_collect_modules_task(
        const lyric_build::BuildGeneration &generation,
        const lyric_build::TaskKey &key,
        std::weak_ptr<lyric_build::BuildState> buildState,
        std::shared_ptr<tempo_tracing::TraceSpan> span);
}

#endif // ZURI_BUILD_COLLECT_MODULES_TASK_H
