#ifndef ZURI_BUILD_COLLECT_MODULES_TASK_H
#define ZURI_BUILD_COLLECT_MODULES_TASK_H

#include <absl/container/flat_hash_set.h>

#include <lyric_build/base_task.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_compiler/lyric_compiler.h>
#include <lyric_parser/lyric_parser.h>

class CollectModulesTask : public lyric_build::BaseTask {

public:
    CollectModulesTask(
        const tempo_utils::UUID &generation,
        const lyric_build::TaskKey &key,
        std::shared_ptr<tempo_tracing::TraceSpan> span);

    tempo_utils::Result<std::string> configureTask(
        const lyric_build::TaskSettings *config,
        lyric_build::AbstractFilesystem *virtualFilesystem) override;
    tempo_utils::Result<absl::flat_hash_set<lyric_build::TaskKey>> checkDependencies() override;
    Option<tempo_utils::Status> runTask(
        const std::string &taskHash,
        const absl::flat_hash_map<lyric_build::TaskKey,lyric_build::TaskState> &depStates,
        lyric_build::BuildState *generation) override;

private:
    absl::flat_hash_set<lyric_build::TaskKey> m_collectTargets;

    absl::flat_hash_map<tempo_utils::UrlPath,lyric_build::ArtifactId> m_artifactsToLink;
    absl::flat_hash_set<tempo_utils::UrlPath> m_modulesFound;

    tempo_utils::Status configure(
        const lyric_build::TaskSettings *config,
        lyric_build::AbstractFilesystem *virtualFilesystem);
    tempo_utils::Status collectModules(
        const std::string &taskHash,
        const absl::flat_hash_map<lyric_build::TaskKey,lyric_build::TaskState> &depStates,
        lyric_build::BuildState *generation);
};

lyric_build::BaseTask *new_collect_modules_task(
    const tempo_utils::UUID &generation,
    const lyric_build::TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span);

#endif // ZURI_BUILD_COLLECT_MODULES_TASK_H
