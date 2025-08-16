
#include <lyric_build/base_task.h>
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_conversions.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/metadata_writer.h>
#include <lyric_build/task_hasher.h>
#include <lyric_common/common_conversions.h>
#include <lyric_common/common_types.h>
#include <zuri_packager/package_attrs.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_utils/log_message.h>
#include <zuri_build/collect_modules_task.h>

zuri_build::CollectModulesTask::CollectModulesTask(
    const tempo_utils::UUID &generation,
    const lyric_build::TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, span)
{
}

tempo_utils::Status
zuri_build::CollectModulesTask::configure(
    const lyric_build::TaskSettings *config,
    lyric_build::AbstractFilesystem *virtualFilesystem)
{
    auto taskId = getId();

    //
    // config below comes only from the task section, it is not resolved from domain or global sections
    //

    auto taskSection = config->getTaskSection(taskId);

    tempo_config::UrlPathParser modulePathParser;
    tempo_config::SeqTParser modulePathsListParser(&modulePathParser);

    std::vector<tempo_utils::UrlPath> modulePathsList;
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
        modulePathsList, modulePathsListParser, taskSection, "modulePaths"));

    if (modulePathsList.empty())
        return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kInvalidConfiguration,
            "task {} has no modules specified for collection", getKey().toString());

    for (const auto &modulePath : modulePathsList) {
        lyric_build::TaskKey provideModuleKey("provide_module", modulePath.toString());
        m_collectTargets.insert(std::move(provideModuleKey));
    }

    TU_LOG_V << "collect targets:" << m_collectTargets;

    return {};
}

tempo_utils::Result<std::string>
zuri_build::CollectModulesTask::configureTask(
    const lyric_build::TaskSettings *config,
    lyric_build::AbstractFilesystem *virtualFilesystem)
{
    auto merged = config->merge(lyric_build::TaskSettings({}, {}, {{getId(), getParams()}}));
    TU_RETURN_IF_NOT_OK (configure(&merged, virtualFilesystem));
    return lyric_build::TaskHasher::uniqueHash();
}

tempo_utils::Result<absl::flat_hash_set<lyric_build::TaskKey>>
zuri_build::CollectModulesTask::checkDependencies()
{
    return m_collectTargets;
}

tempo_utils::Status
zuri_build::CollectModulesTask::collectModules(
    const std::string &taskHash,
    const absl::flat_hash_map<lyric_build::TaskKey,lyric_build::TaskState> &depStates,
    lyric_build::BuildState *buildState)
{
    auto cache = buildState->getCache();

    absl::flat_hash_set<lyric_common::ModuleLocation> modulesNeeded;

    for (const auto &dep : depStates) {
        const auto &taskKey = dep.first;
        const auto &taskState = dep.second;

        m_collectTargets.erase(taskKey);

        // if the target state is not completed, then fail the task
        if (taskState.getStatus() != lyric_build::TaskState::Status::COMPLETED)
            return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kTaskFailure,
                "dependent task {} did not complete", taskKey.toString());

        auto hash = taskState.getHash();
        if (hash.empty())
            return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kBuildInvariant,
                "dependent task {} has invalid hash", taskKey.toString());

        lyric_build::TraceId artifactTrace(hash, taskKey.getDomain(), taskKey.getId());
        auto generation = cache->loadTrace(artifactTrace);

        std::vector<lyric_build::ArtifactId> objectArtifacts;

        // find dep object
        lyric_build::MetadataWriter objectFilterWriter;
        TU_RETURN_IF_NOT_OK (objectFilterWriter.configure());
        objectFilterWriter.putAttr(lyric_build::kLyricBuildContentType, std::string(lyric_common::kObjectContentType));
        lyric_build::LyricMetadata objectFilter;
        TU_ASSIGN_OR_RETURN (objectFilter, objectFilterWriter.toMetadata());
        TU_ASSIGN_OR_RETURN (objectArtifacts, cache->findArtifacts(generation, hash, {}, objectFilter));

        if (objectArtifacts.size() != 1)
            return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kBuildInvariant,
                "expected 1 object artifact in dependent task {} but found {}",
                taskKey.toString(), objectArtifacts.size());
        auto objectDepId = objectArtifacts.front();

        lyric_build::LyricMetadata objectMetadata;
        TU_ASSIGN_OR_RETURN (objectMetadata, cache->loadMetadataFollowingLinks(objectDepId));
        auto objectMetaRoot = objectMetadata.getMetadata();

        lyric_common::ModuleLocation moduleLocation;
        TU_RETURN_IF_NOT_OK (objectMetaRoot.parseAttr(lyric_build::kLyricBuildModuleLocation, moduleLocation));

        // check whether an object already exists for the specified path
        auto objectDepPath = objectDepId.getLocation().toPath();
        if (m_artifactsToLink.contains(objectDepPath))
            return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kBuildInvariant,
                "encountered duplicate object {} in dependent task {}",
                objectDepPath.toString(), taskKey.toString());

        m_artifactsToLink[objectDepPath] = objectDepId;

        // load the object
        std::shared_ptr<const tempo_utils::ImmutableBytes> content;
        TU_ASSIGN_OR_RETURN (content, cache->loadContentFollowingLinks(objectDepId));
        lyric_object::LyricObject object(content);
        auto objectRoot = object.getObject();

        // add unencountered relative imports to the list of modules needed
        for (int i = 0; i < objectRoot.numImports(); i++) {
            auto importLocation = objectRoot.getImport(i).getImportLocation();
            if (importLocation.isRelative()) {
                auto importPath = importLocation.getPath();
                if (!m_artifactsToLink.contains(importPath)) {
                    modulesNeeded.insert(importLocation);
                }
            }
        }

        // check for dep plugin
        if (!object.getObject().hasPlugin())
            continue;

        std::vector<lyric_build::ArtifactId> pluginArtifacts;

        // find dep plugin
        auto objectPluginLocation = object.getObject().getPlugin().getPluginLocation();

        lyric_build::MetadataWriter pluginFilterWriter;
        TU_RETURN_IF_NOT_OK (pluginFilterWriter.configure());
        pluginFilterWriter.putAttr(lyric_build::kLyricBuildContentType, std::string(lyric_common::kPluginContentType));
        lyric_build::LyricMetadata pluginFilter;
        TU_ASSIGN_OR_RETURN (pluginFilter, pluginFilterWriter.toMetadata());
        TU_ASSIGN_OR_RETURN (pluginArtifacts, cache->findArtifacts(generation, hash, {}, pluginFilter));

        if (pluginArtifacts.size() != 1)
            return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kBuildInvariant,
                "expected 1 plugin artifact in dependent task {} but found {}",
                taskKey.toString(), pluginArtifacts.size());
        auto pluginDepId = pluginArtifacts.front();

        // check whether an plugin already exists for the specified path
        auto pluginDepPath = pluginDepId.getLocation().toPath();
        if (m_artifactsToLink.contains(pluginDepPath)) {
            auto &existingPluginArtifact = m_artifactsToLink.at(pluginDepPath);
            if (existingPluginArtifact != pluginDepId)
                return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kBuildInvariant,
                    "encountered duplicate plugin {} in dependent task {}",
                    pluginDepPath.toString(), taskKey.toString());
        }

        m_artifactsToLink[pluginDepPath] = pluginDepId;
    }

    return {};
}

Option<tempo_utils::Status>
zuri_build::CollectModulesTask::runTask(
    const std::string &taskHash,
    const absl::flat_hash_map<lyric_build::TaskKey,lyric_build::TaskState> &depStates,
    lyric_build::BuildState *buildState)
{
    // collect the artifacts from each task dependency
    auto status = collectModules(taskHash, depStates, buildState);
    if (status.notOk())
        return Option(status);

    // if additional targets were discovered during collection then we are not finished
    if (!m_collectTargets.empty())
        return {};

    // otherwise collection is done, so link all collected artifacts
    auto cache = buildState->getCache();
    auto generation = getGeneration();
    for (const auto &entry : m_artifactsToLink) {
        lyric_build::ArtifactId dstId(generation, taskHash, entry.first);
        const auto &depId = entry.second;
        status = cache->linkArtifact(dstId, depId);
        if (status.notOk())
            return Option(status);
    }

    return Option(tempo_utils::Status{});
}

lyric_build::BaseTask *
zuri_build::new_collect_modules_task(
    const tempo_utils::UUID &generation,
    const lyric_build::TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new CollectModulesTask(generation, key, span);
}