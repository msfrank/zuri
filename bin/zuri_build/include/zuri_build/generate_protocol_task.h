#ifndef ZURI_BUILD_GENERATE_PROTOCOL_TASK_H
#define ZURI_BUILD_GENERATE_PROTOCOL_TASK_H

#include <lyric_build/base_task.h>

namespace zuri_build {

    class GenerateProtocolTask : public lyric_build::BaseTask {
    public:
        GenerateProtocolTask(
            const lyric_build::BuildGeneration &generation,
            const lyric_build::TaskKey &key,
            std::weak_ptr<lyric_build::BuildState> buildState,
            std::shared_ptr<tempo_tracing::TraceSpan> span);

        tempo_utils::Status configureTask(const lyric_build::TaskSettings &taskSettings) override;
        tempo_utils::Status deduplicateTask(lyric_build::TaskHash &taskHash) override;
        tempo_utils::Status runTask(lyric_build::TempDirectory *tempDirectory) override;

    private:
        lyric_common::ModuleLocation m_moduleLocation;
        lyric_build::Resource m_resource;
        std::string m_name;
        lyric_object::PortType m_type;
        lyric_object::CommunicationType m_direction;
        lyric_common::SymbolUrl m_sendSymbol;
        lyric_common::SymbolUrl m_receiveSymbol;

        tempo_utils::Status configure(
            const lyric_build::TaskSettings *config,
            lyric_build::AbstractVirtualFilesystem *virtualFilesystem);
        tempo_utils::Status generateProtocol(
            const std::string &taskHash,
            const absl::flat_hash_map<lyric_build::TaskKey,lyric_build::TaskState> &depStates,
            lyric_build::BuildState *buildState);
    };

    lyric_build::BaseTask *new_generate_protocol_task(
        const lyric_build::BuildGeneration &generation,
        const lyric_build::TaskKey &key,
        std::weak_ptr<lyric_build::BuildState> buildState,
        std::shared_ptr<tempo_tracing::TraceSpan> span);
}

#endif // ZURI_BUILD_GENERATE_PROTOCOL_TASK_H
