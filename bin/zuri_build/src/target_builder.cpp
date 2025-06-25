
#include <tempo_command/command_help.h>
#include <zuri_build/target_builder.h>
#include <zuri_build/target_writer.h>

TargetBuilder::TargetBuilder(
    std::shared_ptr<BuildGraph> buildGraph,
    lyric_build::LyricBuilder *builder,
    const std::filesystem::path &installRoot)
    : m_buildGraph(std::move(buildGraph)),
      m_builder(builder),
      m_installRoot(installRoot)
{
    TU_ASSERT (m_buildGraph != nullptr);
    TU_ASSERT (m_builder != nullptr);
    TU_ASSERT (!m_installRoot.empty());
}

tempo_utils::Result<std::filesystem::path>
TargetBuilder::buildTarget(const std::string &targetName)
{
    std::vector<std::string> targetBuildOrder;
    TU_ASSIGN_OR_RETURN (targetBuildOrder, m_buildGraph->calculateBuildOrder(targetName));

    std::filesystem::path targetPath;

    auto targetStore = m_buildGraph->getTargetStore();
    for (const auto &nextTarget : targetBuildOrder) {
        const auto &nextEntry = targetStore->getTarget(nextTarget);
        switch (nextEntry.type) {
            case TargetEntryType::Program: {
                TU_ASSIGN_OR_RETURN (targetPath, buildProgramTarget(nextTarget, nextEntry));
                break;
            }
            case TargetEntryType::Library: {
                TU_ASSIGN_OR_RETURN (targetPath, buildLibraryTarget(nextTarget, nextEntry));
                break;
            }
            default:
                return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kConfigInvariant,
                    "invalid type for build target {}", nextTarget);
        }
    }

    return targetPath;
}

tempo_utils::Result<std::filesystem::path>
TargetBuilder::buildProgramTarget(const std::string &targetName, const TargetEntry &programTarget)
{
    TU_ASSERT (programTarget.type == TargetEntryType::Program);
    return {};
}

tempo_utils::Result<std::filesystem::path>
TargetBuilder::buildLibraryTarget(const std::string &targetName, const TargetEntry &libraryTarget)
{
    TU_ASSERT (libraryTarget.type == TargetEntryType::Library);

    // declare the build tasks
    lyric_build::TaskId collectModules("collect_modules", targetName);
    absl::flat_hash_map<std::string, tempo_config::ConfigNode> collectModulesOverrides;

    std::vector<tempo_config::ConfigNode> modulePaths;
    for (const auto &libraryModule : libraryTarget.modules) {
        tempo_config::ConfigValue modulePath(libraryModule.getPath().toString());
        modulePaths.push_back(std::move(modulePath));
    }
    collectModulesOverrides["modulePaths"] = tempo_config::ConfigSeq(modulePaths);

    absl::flat_hash_map<lyric_build::TaskId, tempo_config::ConfigMap> taskOverrides;
    taskOverrides[collectModules] = tempo_config::ConfigMap(collectModulesOverrides);

    // run the build
    lyric_build::TargetComputationSet targetComputationSet;
    TU_ASSIGN_OR_RETURN (targetComputationSet, m_builder->computeTargets({collectModules},
        lyric_build::TaskSettings({}, {}, taskOverrides)));

    auto targetComputation = targetComputationSet.getTarget(collectModules);
    if (targetComputation.getState().getStatus() != lyric_build::TaskState::Status::COMPLETED) {
        auto diagnostics = targetComputationSet.getDiagnostics();
        diagnostics->printDiagnostics();
        return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
            "failed to build target '{}'", targetName);
    }

    // construct the target writer
    TargetWriter targetWriter(m_installRoot, libraryTarget.specifier);
    TU_RETURN_IF_NOT_OK (targetWriter.configure());

    auto cache = m_builder->getCache();
    std::vector<lyric_build::ArtifactId> targetArtifacts;

    // write collected modules
    auto collectModulesComputation = targetComputationSet.getTarget(collectModules);
    auto collectModulesState = collectModulesComputation.getState();

    TU_ASSIGN_OR_RETURN (targetArtifacts, cache->findArtifacts(
        collectModulesState.getGeneration(), collectModulesState.getHash(), {}, {}));
    for (const auto &artifactId : targetArtifacts) {
        lyric_build::LyricMetadata metadata;
        TU_ASSIGN_OR_RETURN (metadata, cache->loadMetadataFollowingLinks(artifactId));
        std::shared_ptr<const tempo_utils::ImmutableBytes> content;
        TU_ASSIGN_OR_RETURN (content, cache->loadContentFollowingLinks(artifactId));
        targetWriter.writeModule(artifactId.getLocation().toPath(), metadata, content);
    }

    return targetWriter.writeTarget();
}