
#include <tempo_command/command_help.h>
#include <tempo_config/config_builder.h>
#include <zuri_build/build_result.h>
#include <zuri_build/target_builder.h>
#include <zuri_build/target_writer.h>

zuri_build::TargetBuilder::TargetBuilder(
    std::shared_ptr<zuri_distributor::RuntimeEnvironment> runtimeEnvironment,
    std::shared_ptr<zuri_tooling::BuildGraph> buildGraph,
    lyric_build::LyricBuilder *builder,
    absl::flat_hash_map<std::string,tempo_utils::Url> &&targetBases,
    const std::filesystem::path &installRoot)
    : m_runtimeEnvironment(std::move(runtimeEnvironment)),
      m_buildGraph(std::move(buildGraph)),
      m_builder(builder),
      m_targetBases(std::move(targetBases)),
      m_installRoot(installRoot)
{
    TU_ASSERT (m_runtimeEnvironment != nullptr);
    TU_ASSERT (m_buildGraph != nullptr);
    TU_ASSERT (m_builder != nullptr);
    TU_ASSERT (!m_installRoot.empty());
}

tempo_utils::Result<std::filesystem::path>
zuri_build::TargetBuilder::buildTarget(const std::string &targetName)
{
    auto targetStore = m_buildGraph->getTargetStore();

    // make a fresh copy of target bases
    auto targetBases = m_targetBases;

    std::filesystem::path currTargetPath;

    // determine the order in which to build the specified target and all of its dependent targets
    std::vector<std::string> targetBuildOrder;
    TU_ASSIGN_OR_RETURN (targetBuildOrder, m_buildGraph->calculateBuildOrder(targetName));

    // process each target in order
    for (const auto &currTargetName : targetBuildOrder) {
        const auto &currEntry = targetStore->getTarget(currTargetName);

        // package targets are processed during initialization so there is nothing to do
        if (currEntry->type == zuri_tooling::TargetEntryType::Package) {
            continue;
        }

        // construct the shortcut resolver for the target
        auto targetShortcuts = std::make_shared<lyric_importer::ShortcutResolver>();
        for (const auto &dependsName : currEntry->depends) {
            auto entry = targetBases.find(dependsName);
            if (entry == targetBases.cend())
                return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                    "unknown dependent '{}' for target {}", dependsName, currTargetName);
            TU_RETURN_IF_NOT_OK (targetShortcuts->insertShortcut(entry->first, entry->second));
        }

        // build the target
        switch (currEntry->type) {
            case zuri_tooling::TargetEntryType::Program:
                TU_ASSIGN_OR_RETURN (currTargetPath, buildProgramTarget(currTargetName, currEntry, targetShortcuts));
                break;
            case zuri_tooling::TargetEntryType::Library:
                TU_ASSIGN_OR_RETURN (currTargetPath, buildLibraryTarget(currTargetName, currEntry, targetShortcuts));
                break;
            default:
                return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kConfigInvariant,
                    "invalid type for build target {}", currTargetName);
        }

        // if we are processing a dependent target then install it in the targets cache
        if (currTargetName != targetName) {
            std::shared_ptr<zuri_packager::PackageReader> packageReader;
            TU_ASSIGN_OR_RETURN (packageReader, zuri_packager::PackageReader::open(currTargetPath));
            zuri_packager::PackageSpecifier specifier;
            TU_ASSIGN_OR_RETURN (specifier, packageReader->readPackageSpecifier());

            // if target exists in package cache then remove it first
            // TODO: if package is unchanged then don't reinstall it
            if (m_runtimeEnvironment->containsPackage(specifier)) {
                TU_RETURN_IF_NOT_OK (m_runtimeEnvironment->removePackage(specifier));
            }
            TU_RETURN_IF_STATUS (m_runtimeEnvironment->installPackage(packageReader));

            // add package base for target
            targetBases[currTargetName] = specifier.toUrl();
        }
    }

    return currTargetPath;
}

tempo_utils::Result<std::filesystem::path>
zuri_build::TargetBuilder::buildProgramTarget(
    const std::string &targetName,
    std::shared_ptr<const zuri_tooling::TargetEntry> targetEntry,
    std::shared_ptr<lyric_importer::ShortcutResolver> targetShortcuts)
{
    TU_ASSERT (targetEntry->type == zuri_tooling::TargetEntryType::Program);
    const auto &program = std::get<zuri_tooling::TargetEntry::Program>(targetEntry->target);

    auto targetSettings = targetEntry->settings;

    // declare the build tasks
    lyric_build::TaskId collectModules("collect_modules", targetName);
    auto collectModulesOverridesBuilder = tempo_config::startMap();

    auto modulePathsBuilder = tempo_config::startSeq()
        .append(tempo_config::valueNode(program.main.getPath().toString()));
    for (const auto &programModule : program.modules) {
        modulePathsBuilder = modulePathsBuilder
            .append(tempo_config::valueNode(programModule.getPath().toString()));
    }
    collectModulesOverridesBuilder = collectModulesOverridesBuilder
        .put("modulePaths", modulePathsBuilder.buildNode());

    absl::flat_hash_map<lyric_build::TaskId, tempo_config::ConfigMap> taskOverrides;
    taskOverrides[collectModules] = collectModulesOverridesBuilder.buildMap();
    auto settingsOverrides = lyric_build::TaskSettings({}, {}, taskOverrides);

    lyric_build::ComputeTargetOverrides overrides;
    overrides.settings = targetSettings.merge(settingsOverrides);
    overrides.shortcuts = targetShortcuts;

    // run the build
    lyric_build::TargetComputationSet targetComputationSet;
    TU_ASSIGN_OR_RETURN (targetComputationSet, m_builder->computeTarget(collectModules, overrides));

    auto targetComputation = targetComputationSet.getTarget(collectModules);
    if (targetComputation.getState().getStatus() != lyric_build::TaskState::Status::COMPLETED) {
        auto diagnostics = targetComputationSet.getDiagnostics();
        diagnostics->printDiagnostics();
        return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
            "failed to build target '{}'", targetName);
    }

    // construct the target writer
    TargetWriter targetWriter(m_runtimeEnvironment, m_installRoot, program.specifier);
    TU_RETURN_IF_NOT_OK (targetWriter.configure());

    // set the main location
    targetWriter.setProgramMain(program.main);

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
    std::shared_ptr<const zuri_tooling::TargetEntry> targetEntry,
    std::shared_ptr<lyric_importer::ShortcutResolver> targetShortcuts)
{
    TU_ASSERT (targetEntry->type == zuri_tooling::TargetEntryType::Library);
    const auto &library = std::get<zuri_tooling::TargetEntry::Library>(targetEntry->target);

    auto targetSettings = targetEntry->settings;

    // declare the build tasks
    lyric_build::TaskId collectModules("collect_modules", targetName);
    auto collectModulesOverridesBuilder = tempo_config::startMap();

    auto modulePathsBuilder = tempo_config::startSeq();
    for (const auto &libraryModule : library.modules) {
        modulePathsBuilder = modulePathsBuilder
            .append(tempo_config::valueNode(libraryModule.getPath().toString()));
    }
    collectModulesOverridesBuilder = collectModulesOverridesBuilder
        .put("modulePaths", modulePathsBuilder.buildNode());

    absl::flat_hash_map<lyric_build::TaskId, tempo_config::ConfigMap> taskOverrides;
    taskOverrides[collectModules] = collectModulesOverridesBuilder.buildMap();
    auto settingsOverrides = lyric_build::TaskSettings({}, {}, taskOverrides);

    lyric_build::ComputeTargetOverrides overrides;
    overrides.settings = targetSettings.merge(settingsOverrides);
    overrides.shortcuts = targetShortcuts;

    // run the build
    lyric_build::TargetComputationSet targetComputationSet;
    TU_ASSIGN_OR_RETURN (targetComputationSet, m_builder->computeTarget(collectModules, overrides));

    auto targetComputation = targetComputationSet.getTarget(collectModules);
    if (targetComputation.getState().getStatus() != lyric_build::TaskState::Status::COMPLETED) {
        auto diagnostics = targetComputationSet.getDiagnostics();
        diagnostics->printDiagnostics();
        return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
            "failed to build target '{}'", targetName);
    }

    // construct the target writer
    TargetWriter targetWriter(m_runtimeEnvironment, m_installRoot, library.specifier);
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
        TU_RETURN_IF_NOT_OK (targetWriter.writeModule(artifactId.getLocation().toPath(), metadata, content));
    }

    return targetWriter.writeTarget();
}