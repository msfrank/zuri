
#include <lyric_build/base_task.h>
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_conversions.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/metadata_writer.h>
#include <lyric_build/task_hasher.h>
#include <lyric_build/task_utils.h>
#include <lyric_common/common_conversions.h>
#include <lyric_common/common_types.h>
#include <lyric_runtime/connection.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_utils/log_message.h>
#include <tempo_utils/platform.h>
#include <zuri_build/collect_modules_task.h>

zuri_build::CollectModulesTask::CollectModulesTask(
    const lyric_build::BuildGeneration &generation,
    const lyric_build::TaskKey &key,
            std::weak_ptr<lyric_build::BuildState> buildState,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, std::move(buildState), std::move(span)),
      m_phase(Phase::Initial)
{
}

tempo_utils::Status
zuri_build::CollectModulesTask::initial(const lyric_build::TaskSettings &settings)
{
    auto taskId = getId();

    m_phase = Phase::CollectModules;

    //
    // config below comes only from the task section, it is not resolved from domain or global sections
    //

    auto taskSection = settings.getTaskSection(taskId);

    tempo_config::UrlPathParser objectPathParser;
    tempo_config::SeqTParser objectPathsListParser(&objectPathParser);

    std::vector<tempo_utils::UrlPath> objectPathsList;
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
        objectPathsList, objectPathsListParser, taskSection, "objectPaths"));

    if (objectPathsList.empty())
        return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kInvalidConfiguration,
            "task {} has no objects specified for collection", getKey().toString());

    for (const auto &objectPath : objectPathsList) {
        lyric_build::TaskKey provideObjectKey("provide_object", objectPath.toString());
        requestDependency(provideObjectKey);
    }

    return {};
}

tempo_utils::Status
zuri_build::CollectModulesTask::collectObject(
    const tempo_utils::UrlPath &objectArtifactPath,
    const lyric_build::TaskKey &taskKey,
    std::shared_ptr<lyric_build::AbstractArtifactCache> &artifactCache)
{
    // check whether an object already exists for the specified path
    if (m_artifactsToLink.contains(objectArtifactPath))
        return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kBuildInvariant,
            "encountered duplicate object {} in dependent task {}",
            objectArtifactPath.toString(), taskKey.toString());

    m_artifactsToLink[objectArtifactPath] = taskKey;

    // load the object
    std::shared_ptr<const tempo_utils::ImmutableBytes> content;
    TU_ASSIGN_OR_RETURN (content, getContent(taskKey, objectArtifactPath, true));
    lyric_object::LyricObject object(content);

    // add unencountered relative imports to the list of dependencies
    for (int i = 0; i < object.numImports(); i++) {
        auto importLocation = object.getImport(i).getImportLocation();
        if (!importLocation.isRelative())
            continue;
        auto importPath = importLocation.getPath();
        if (m_artifactsToLink.contains(importPath))
            continue;
        lyric_build::TaskKey neededTarget("provide_module", importPath.toString());
        if (m_collectTargets.contains(neededTarget))
            continue;

        requestDependency(neededTarget);
    }

    // if object specifies a plugin then add plugin to list of dependencies
    if (object.hasPlugin()) {
        auto walker = object.getPlugin();
        auto pluginLocation = walker.getPluginLocation();
        auto pluginTarget = lyric_build::TaskKey("provide_plugin", pluginLocation.getPath().toString());
        requestDependency(pluginTarget);
    }

    return {};
}

tempo_utils::Status
zuri_build::CollectModulesTask::collectPlugin(
    const tempo_utils::UrlPath &pluginArtifactPath,
    const lyric_build::TaskKey &taskKey,
    std::shared_ptr<lyric_build::AbstractArtifactCache> &artifactCache)
{
    // check whether a plugin already exists for the specified path
    if (m_artifactsToLink.contains(pluginArtifactPath))
        return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kBuildInvariant,
            "encountered duplicate plugin {} in dependent task {}",
            pluginArtifactPath.toString(), taskKey.toString());

    m_artifactsToLink[pluginArtifactPath] = taskKey;

    return {};
}

tempo_utils::Status
zuri_build::CollectModulesTask::collectModules(const lyric_build::TaskSettings &settings)
{
    auto buildState = getBuildState();
    auto artifactCache = buildState->getArtifactCache();

    for (auto it = completedBegin(); it != completedEnd(); ++it) {
        const auto &taskKey = it->first;

        //
        if (m_collectTargets.contains(taskKey))
            continue;

        //
        if (taskKey.getDomain() == "provide_object") {
            m_collectTargets[taskKey] = CollectType::Object;

            auto moduleLocation = lyric_common::ModuleLocation::fromString(taskKey.getId());
            tempo_utils::UrlPath objectArtifactPath;
            TU_ASSIGN_OR_RETURN (objectArtifactPath, lyric_build::convert_module_location_to_artifact_path(
                moduleLocation, lyric_common::kObjectFileDotSuffix));

            TU_RETURN_IF_NOT_OK (collectObject(objectArtifactPath, taskKey, artifactCache));

        } else if (taskKey.getDomain() == "provide_plugin") {
            m_collectTargets[taskKey] = CollectType::Plugin;

            auto moduleLocation = lyric_common::ModuleLocation::fromString(taskKey.getId());
            tempo_utils::UrlPath pluginArtifactPath;
            TU_ASSIGN_OR_RETURN (pluginArtifactPath, lyric_build::convert_module_location_to_artifact_path(
                moduleLocation, absl::StrCat(
                    ".", tempo_utils::sharedLibraryPlatformId(), tempo_utils::sharedLibraryFileDotSuffix())));

            TU_RETURN_IF_NOT_OK (collectPlugin(pluginArtifactPath, taskKey, artifactCache));

        } else {
            logWarn("ignoring artifacts from task {}", taskKey.toString());
        }
    }

    if (dependenciesEmpty()) {
        m_phase = Phase::Complete;
    }

    return {};
}

tempo_utils::Status
zuri_build::CollectModulesTask::configureTask(const lyric_build::TaskSettings &taskSettings)
{
    auto taskId = getId();
    auto settings = taskSettings.merge(lyric_build::TaskSettings({}, {}, {{taskId, getParams()}}));

    switch (m_phase) {
        case Phase::Initial:
            return initial(settings);
        case Phase::CollectModules:
            return collectModules(settings);
        case Phase::Complete:
            return {};
    }
}

tempo_utils::Status
zuri_build::CollectModulesTask::deduplicateTask(lyric_build::TaskHash &taskHash)
{
    lyric_build::TaskHasher hasher(getKey());
    hasher.hashTask(this);
    taskHash = hasher.finish();
    return {};
}

tempo_utils::Status
zuri_build::CollectModulesTask::runTask(lyric_build::TempDirectory *tempDirectory)
{
    if (m_collectTargets.empty())
        return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kTaskFailure,
            "task {} has no modules specified for collection", getKey().toString());

    TU_LOG_V << "linking collected modules for " << getId().toString();

    auto buildState = getBuildState();
    auto artifactCache = buildState->getArtifactCache();

    // link all collected artifacts
    auto generation = getGeneration();
    for (const auto &entry : m_artifactsToLink) {
        TU_RETURN_IF_NOT_OK (linkArtifact(entry.second, entry.first));
        TU_LOG_V << "linking " << entry.first;
    }

    return {};
}

lyric_build::BaseTask *
zuri_build::new_collect_modules_task(
    const lyric_build::BuildGeneration &generation,
    const lyric_build::TaskKey &key,
    std::weak_ptr<lyric_build::BuildState> buildState,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new CollectModulesTask(generation, key, std::move(buildState), std::move(span));
}