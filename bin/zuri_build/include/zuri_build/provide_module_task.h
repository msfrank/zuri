#ifndef ZURI_BUILD_PROVIDE_MODULE_TASK_H
#define ZURI_BUILD_PROVIDE_MODULE_TASK_H

#include <lyric_assembler/object_state.h>
#include <lyric_build/base_task.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_settings.h>

#include "provider_task_operations.h"

namespace zuri_build {

    class ProvideModuleTask : public lyric_build::BaseTask, ProviderTaskOperations {

        enum class Phase {
            Initial,
            ProvidePlugin,
            Complete,
        };

    public:
        ProvideModuleTask(
            const lyric_build::BuildGeneration &generation,
            const lyric_build::TaskKey &key,
            std::weak_ptr<lyric_build::BuildState> buildState,
            std::shared_ptr<tempo_tracing::TraceSpan> span);

        tempo_utils::Status configureTask(const lyric_build::TaskSettings &taskSettings) override;
        tempo_utils::Status deduplicateTask(lyric_build::TaskHash &taskHash) override;
        tempo_utils::Status runTask(lyric_build::TempDirectory *tempDirectory) override;

    private:
        lyric_common::ModuleLocation m_moduleLocation;
        lyric_build::TaskKey m_objectTarget;
        ExistingArtifact m_existingObject;
        lyric_build::TaskKey m_pluginTarget;
        lyric_common::ModuleLocation m_pluginLocation;
        std::shared_ptr<const tempo_utils::ImmutableBytes> m_existingContent;
        Phase m_phase;

        tempo_utils::Status initial(const lyric_build::TaskSettings &settings);
        tempo_utils::Status providePlugin(const lyric_build::TaskSettings &settings);
    };

    lyric_build::BaseTask *new_provide_module_task(
        const lyric_build::BuildGeneration &generation,
        const lyric_build::TaskKey &key,
        std::weak_ptr<lyric_build::BuildState> buildState,
        std::shared_ptr<tempo_tracing::TraceSpan> span);
}

#endif // ZURI_BUILD_PROVIDE_MODULE_TASK_H
