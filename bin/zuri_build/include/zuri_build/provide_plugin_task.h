#ifndef ZURI_BUILD_PROVIDE_PLUGIN_TASK_H
#define ZURI_BUILD_PROVIDE_PLUGIN_TASK_H

#include <lyric_assembler/object_state.h>
#include <lyric_build/base_task.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_settings.h>

#include "provider_task_operations.h"

namespace zuri_build {

    class ProvidePluginTask : public lyric_build::BaseTask, ProviderTaskOperations {

    public:
        ProvidePluginTask(
            const lyric_build::BuildGeneration &generation,
            const lyric_build::TaskKey &key,
            std::weak_ptr<lyric_build::BuildState> buildState,
            std::shared_ptr<tempo_tracing::TraceSpan> span);

        tempo_utils::Status configureTask(const lyric_build::TaskSettings &taskSettings) override;
        tempo_utils::Status deduplicateTask(lyric_build::TaskHash &taskHash) override;
        tempo_utils::Status runTask(lyric_build::TempDirectory *tempDirectory) override;

    private:
        lyric_common::ModuleLocation m_pluginLocation;
        lyric_build::TaskKey m_provideTarget;
        ExistingArtifact m_existingPlugin;
        std::shared_ptr<const tempo_utils::ImmutableBytes> m_existingContent;
    };

    lyric_build::BaseTask *new_provide_plugin_task(
        const lyric_build::BuildGeneration &generation,
        const lyric_build::TaskKey &key,
        std::weak_ptr<lyric_build::BuildState> buildState,
        std::shared_ptr<tempo_tracing::TraceSpan> span);
}

#endif // ZURI_BUILD_PROVIDE_PLUGIN_TASK_H