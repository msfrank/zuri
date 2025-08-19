
#include <tempo_command/command_help.h>
#include <tempo_config/config_builder.h>
#include <zuri_build/target_builder.h>
#include <zuri_build/target_writer.h>

zuri_build::TargetBuilder::TargetBuilder(
    std::shared_ptr<zuri_tooling::BuildGraph> buildGraph,
    lyric_build::LyricBuilder *builder,
    std::shared_ptr<lyric_importer::ShortcutResolver> shortcutResolver,
    std::shared_ptr<zuri_distributor::PackageCache> targetPackageCache,
    const std::filesystem::path &installRoot)
    : m_buildGraph(std::move(buildGraph)),
      m_builder(builder),
      m_shortcutResolver(std::move(shortcutResolver)),
      m_targetPackageCache(std::move(targetPackageCache)),
      m_installRoot(installRoot)
{
    TU_ASSERT (m_buildGraph != nullptr);
    TU_ASSERT (m_builder != nullptr);
    TU_ASSERT (m_targetPackageCache != nullptr);
    TU_ASSERT (!m_installRoot.empty());
}

tempo_utils::Result<std::filesystem::path>
zuri_build::TargetBuilder::buildTarget(
    const std::string &targetName,
    const absl::flat_hash_map<std::string,std::string> &targetShortcuts)
{
    std::vector<std::string> targetBuildOrder;
    TU_ASSIGN_OR_RETURN (targetBuildOrder, m_buildGraph->calculateBuildOrder(targetName));

    std::filesystem::path currTargetPath;
    std::filesystem::path prevTargetPath;
    std::string prevTargetName;

    auto targetStore = m_buildGraph->getTargetStore();

    for (const auto &currTargetName : targetBuildOrder) {

        // if target dependency exists then install it in the target package cache
        if (!prevTargetPath.empty()) {
            std::shared_ptr<zuri_packager::PackageReader> packageReader;
            TU_ASSIGN_OR_RETURN (packageReader, zuri_packager::PackageReader::open(prevTargetPath));
            zuri_packager::PackageSpecifier specifier;
            TU_ASSIGN_OR_RETURN (specifier, packageReader->readPackageSpecifier());

            // if target exists in package cache then remove it first
            if (m_targetPackageCache->containsPackage(specifier)) {
                TU_RETURN_IF_NOT_OK (m_targetPackageCache->removePackage(specifier));
            }
            TU_RETURN_IF_STATUS (m_targetPackageCache->installPackage(packageReader));

            //
            auto entry = targetShortcuts.find(prevTargetName);
            if (entry != targetShortcuts.cend()) {
                const auto &shortcutName = entry->second;
                if (!m_shortcutResolver->hasShortcut(shortcutName)) {
                    auto origin = specifier.toUrlOrigin();
                    TU_RETURN_IF_NOT_OK (m_shortcutResolver->insertShortcut(shortcutName, origin));
                }
            }
        }

        const auto &currEntry = targetStore->getTarget(currTargetName);
        switch (currEntry->type) {
            case zuri_tooling::TargetEntryType::Program: {
                TU_ASSIGN_OR_RETURN (currTargetPath, buildProgramTarget(currTargetName, currEntry));
                break;
            }
            case zuri_tooling::TargetEntryType::Library: {
                TU_ASSIGN_OR_RETURN (currTargetPath, buildLibraryTarget(currTargetName, currEntry));
                break;
            }
            default:
                return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kConfigInvariant,
                    "invalid type for build target {}", currTargetName);
        }
        prevTargetPath = currTargetPath;
        prevTargetName = currTargetName;
    }

    return currTargetPath;
}

tempo_utils::Result<std::filesystem::path>
zuri_build::TargetBuilder::buildProgramTarget(
    const std::string &targetName,
    std::shared_ptr<const zuri_tooling::TargetEntry> programTarget)
{
    TU_ASSERT (programTarget->type == zuri_tooling::TargetEntryType::Program);

    // declare the build tasks
    lyric_build::TaskId collectModules("collect_modules", targetName);
    auto collectModulesOverridesBuilder = tempo_config::startMap();

    auto modulePathsBuilder = tempo_config::startSeq()
        .append(tempo_config::valueNode(programTarget->main.getPath().toString()));
    for (const auto &programModule : programTarget->modules) {
        modulePathsBuilder = modulePathsBuilder
            .append(tempo_config::valueNode(programModule.getPath().toString()));
    }
    collectModulesOverridesBuilder = collectModulesOverridesBuilder
        .put("modulePaths", modulePathsBuilder.buildNode());

    absl::flat_hash_map<lyric_build::TaskId, tempo_config::ConfigMap> taskOverrides;
    taskOverrides[collectModules] = collectModulesOverridesBuilder.buildMap();

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
    TargetWriter targetWriter(m_installRoot, programTarget->specifier);
    TU_RETURN_IF_NOT_OK (targetWriter.configure());

    // set the main location
    targetWriter.setProgramMain(programTarget->main);

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

tempo_utils::Result<std::filesystem::path>
zuri_build::TargetBuilder::buildLibraryTarget(
    const std::string &targetName,
    std::shared_ptr<const zuri_tooling::TargetEntry> libraryTarget)
{
    TU_ASSERT (libraryTarget->type == zuri_tooling::TargetEntryType::Library);

    // declare the build tasks
    lyric_build::TaskId collectModules("collect_modules", targetName);
    auto collectModulesOverridesBuilder = tempo_config::startMap();

    auto modulePathsBuilder = tempo_config::startSeq();
    for (const auto &libraryModule : libraryTarget->modules) {
        modulePathsBuilder = modulePathsBuilder
            .append(tempo_config::valueNode(libraryModule.getPath().toString()));
    }
    collectModulesOverridesBuilder = collectModulesOverridesBuilder
        .put("modulePaths", modulePathsBuilder.buildNode());

    absl::flat_hash_map<lyric_build::TaskId, tempo_config::ConfigMap> taskOverrides;
    taskOverrides[collectModules] = collectModulesOverridesBuilder.buildMap();

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
    TargetWriter targetWriter(m_installRoot, libraryTarget->specifier);
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