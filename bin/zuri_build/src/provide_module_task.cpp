
#include <lyric_build/base_task.h>
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_conversions.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_settings.h>
#include <lyric_build/metadata_writer.h>
#include <lyric_build/task_hasher.h>
#include <lyric_build/task_utils.h>
#include <lyric_common/common_types.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_utils/log_message.h>
#include <tempo_utils/platform.h>
#include <zuri_build/provide_module_task.h>

zuri_build::ProvideModuleTask::ProvideModuleTask(
    const lyric_build::BuildGeneration &generation,
    const lyric_build::TaskKey &key,
    std::weak_ptr<lyric_build::BuildState> buildState,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, std::move(buildState), std::move(span)),
      m_phase(Phase::Initial)
{
}

tempo_utils::Status
zuri_build::ProvideModuleTask::initial(const lyric_build::TaskSettings &settings)
{
    auto taskId = getId();

    auto modulePath = tempo_utils::UrlPath::fromString(taskId.getId());
    if (!modulePath.isValid())
        return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kInvalidConfiguration,
            "task key id {} is not a valid module location", taskId.getId());

    m_phase = Phase::ProvidePlugin;

    m_moduleLocation = lyric_common::ModuleLocation::fromString(modulePath.toString());

    // if task has a provider.config, then use it to configure the task
    if (m_moduleLocation.isRelative()) {
        auto providerConfigPath = m_moduleLocation.getPath();
        auto buildState = getBuildState();
        auto vfs = buildState->getVirtualFilesystem();

        // determine the base path containing source files
        tempo_config::UrlPathParser sourceBasePathParser(tempo_utils::UrlPath{});
        tempo_utils::UrlPath sourceBasePath;
        TU_RETURN_IF_NOT_OK(parse_config(sourceBasePath, sourceBasePathParser,
            settings, taskId, "sourceBasePath"));

        // if the source base path was specified then augment the source path
        if (sourceBasePath.isValid()) {
            providerConfigPath = sourceBasePath.toAbsolute().traverse(providerConfigPath.toRelative());
        }

        Option<lyric_build::Resource> resourceOption;
        TU_ASSIGN_OR_RETURN (resourceOption, findProviderConfig(providerConfigPath, vfs));

        if (resourceOption.hasValue()) {
            auto resource = resourceOption.getValue();
            TU_ASSIGN_OR_RETURN (m_objectTarget, configureProvider(resource, vfs));
            requestDependency(m_objectTarget);
            return {};
        }
    }

    auto taskSection = settings.getTaskSection(taskId);

    // if 'existingObject' setting is defined then use it to configure the task
    if (taskSection.mapContains("existingObject")) {
        auto existingObjectNode = taskSection.mapAt("existingObject");
        TU_ASSIGN_OR_RETURN (m_existingObject, configureExistingArtifact(existingObjectNode));
        return {};
    }

    // otherwise use the default provide target
    m_objectTarget = lyric_build::TaskKey("provide_object", taskId.getId());
    requestDependency(m_objectTarget);

    return {};
}

tempo_utils::Status
zuri_build::ProvideModuleTask::providePlugin(const lyric_build::TaskSettings &settings)
{
    m_phase = Phase::Complete;

    std::shared_ptr<const tempo_utils::ImmutableBytes> objectContent;
    if (m_objectTarget.isValid()) {
        tempo_utils::UrlPath objectArtifactPath;
        TU_ASSIGN_OR_RETURN (objectArtifactPath, lyric_build::convert_module_location_to_artifact_path(
            m_moduleLocation, lyric_common::kObjectFileDotSuffix));
        TU_ASSIGN_OR_RETURN (objectContent, getContent(m_objectTarget, objectArtifactPath, true));
    } else {
        objectContent = m_existingContent;
    }

    lyric_object::LyricObject object(objectContent);

    if (object.hasPlugin()) {
        auto walker = object.getPlugin();
        m_pluginLocation = walker.getPluginLocation();
        m_pluginTarget = lyric_build::TaskKey("provide_plugin", m_pluginLocation.getPath().toString());
        requestDependency(m_pluginTarget);
    }

    return {};
}

tempo_utils::Status
zuri_build::ProvideModuleTask::configureTask(const lyric_build::TaskSettings &taskSettings)
{
    auto taskId = getId();
    auto settings = taskSettings.merge(lyric_build::TaskSettings({}, {}, {{taskId, getParams()}}));

    switch (m_phase) {
        case Phase::Initial:
            return initial(settings);
        case Phase::ProvidePlugin:
            return providePlugin(settings);
        case Phase::Complete:
            return {};
    }
}

tempo_utils::Status
zuri_build::ProvideModuleTask::deduplicateTask(lyric_build::TaskHash &taskHash)
{
    lyric_build::TaskHasher hasher(getKey());
    hasher.hashValue(m_moduleLocation.toString());
    hasher.hashTask(this);

    if (m_existingObject.type != ExistingArtifact::Type::Invalid) {
        auto buildState = getBuildState();
        auto vfs = buildState->getVirtualFilesystem();
        TU_ASSIGN_OR_RETURN (m_existingContent, loadExistingArtifact(m_existingObject, vfs));
        TU_RETURN_IF_NOT_OK (hashExistingArtifact(m_existingObject, m_existingContent, hasher));
    }

    taskHash = hasher.finish();
    return {};
}

// tempo_utils::Status
// zuri_build::ProvideModuleTask::provideObject(
//     const std::string &taskHash,
//     const absl::flat_hash_map<lyric_build::TaskKey,lyric_build::TaskState> &depStates,
//     lyric_build::BuildState *buildState)
// {
//     tempo_utils::UrlPath objectArtifactPath;
//     TU_ASSIGN_OR_RETURN (objectArtifactPath, lyric_build::convert_module_location_to_artifact_path(
//         m_moduleLocation, lyric_common::kObjectFileDotSuffix));
//
//     auto artifactCache = buildState->getArtifactCache();
//
//     // pull forward the object from the completed target
//     TU_ASSERT (depStates.contains(m_objectTarget));
//     const auto &taskState = depStates.at(m_objectTarget);
//
//     // if the target state is not completed, then fail the task
//     if (taskState.getStatus() != lyric_build::TaskState::Status::COMPLETED)
//         return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kTaskFailure,
//             "dependent task {} did not complete", m_objectTarget.toString());
//
//     auto hash = taskState.getHash();
//     if (hash.empty())
//         return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kBuildInvariant,
//             "dependent task {} has invalid hash", m_objectTarget.toString());
//     lyric_build::TraceId artifactTrace(hash, m_objectTarget.getDomain(), m_objectTarget.getId());
//     tempo_utils::UUID generation;
//     TU_ASSIGN_OR_RETURN (generation, artifactCache->loadTrace(artifactTrace));
//
//     // TODO: support task setting to specify the source artifact path explicitly
//     lyric_build::ArtifactId objectSrcId(generation, hash, objectArtifactPath);
//     lyric_build::ArtifactId objectDstId(getGeneration(), taskHash, objectArtifactPath);
//
//     lyric_build::MetadataWriter objectWriter;
//     TU_RETURN_IF_NOT_OK (objectWriter.configure());
//     objectWriter.putAttr(lyric_build::kLyricBuildContentType, std::string(lyric_common::kObjectContentType));
//     objectWriter.putAttr(lyric_build::kLyricBuildModuleLocation, m_moduleLocation);
//     lyric_build::LyricMetadata objectMetadata;
//     TU_ASSIGN_OR_RETURN (objectMetadata, objectWriter.toMetadata());
//
//     TU_RETURN_IF_NOT_OK (artifactCache->linkArtifactOverridingMetadata(objectDstId, objectMetadata, objectSrcId));
//
//     logInfo("linked object at {}", objectDstId.toString());
//
//     // check for plugin
//     std::shared_ptr<const tempo_utils::ImmutableBytes> content;
//     TU_ASSIGN_OR_RETURN (content, artifactCache->loadContentFollowingLinks(objectSrcId));
//     lyric_object::LyricObject object(content);
//
//     if (object.hasPlugin()) {
//         auto pluginLocation = object.getPlugin().getPluginLocation();
//         m_pluginTarget = lyric_build::TaskKey("provide_plugin", pluginLocation.getPath().toString());
//         m_dependencies.insert(m_pluginTarget);
//         m_phase = Phase::PROVIDE_PLUGIN;
//     } else {
//         m_phase = Phase::COMPLETE;
//     }
//
//     return {};
// }
//
// tempo_utils::Status
// zuri_build::ProvideModuleTask::useExisting(
//     const std::string &taskHash,
//     const absl::flat_hash_map<lyric_build::TaskKey,lyric_build::TaskState> &depStates,
//     lyric_build::BuildState *buildState)
// {
//     tempo_utils::UrlPath objectArtifactPath;
//     TU_ASSIGN_OR_RETURN (objectArtifactPath, lyric_build::convert_module_location_to_artifact_path(
//         m_moduleLocation, lyric_common::kObjectFileDotSuffix));
//
//     auto artifactCache = buildState->getArtifactCache();
//     auto vfs = buildState->getVirtualFilesystem();
//
//     std::shared_ptr<const tempo_utils::ImmutableBytes> content;
//     TU_ASSIGN_OR_RETURN (content, loadExistingArtifact(m_existingObject, vfs.get()));
//     lyric_object::LyricObject object(content);
//
//     // declare the artifact
//     lyric_build::ArtifactId objectArtifact(buildState->getGeneration().getUuid(), taskHash, objectArtifactPath);
//     TU_RETURN_IF_NOT_OK (artifactCache->declareArtifact(objectArtifact));
//
//     // serialize the object metadata
//     lyric_build::MetadataWriter objectMetadataWriter;
//     TU_RETURN_IF_NOT_OK (objectMetadataWriter.configure());
//     objectMetadataWriter.putAttr(lyric_build::kLyricBuildContentType, std::string(lyric_common::kObjectContentType));
//     objectMetadataWriter.putAttr(lyric_build::kLyricBuildModuleLocation, m_moduleLocation);
//     lyric_build::LyricMetadata objectMetadata;
//     TU_ASSIGN_OR_RETURN (objectMetadata, objectMetadataWriter.toMetadata());
//
//     // store the object metadata in the build cache
//     TU_RETURN_IF_NOT_OK (artifactCache->storeMetadata(objectArtifact, objectMetadata));
//
//     // store the object content in the build cache
//     TU_RETURN_IF_NOT_OK (artifactCache->storeContent(objectArtifact, content));
//
//     logInfo("stored object at {}", objectArtifact.toString());
//
//     if (object.hasPlugin()) {
//         auto pluginLocation = object.getPlugin().getPluginLocation();
//         m_pluginTarget = lyric_build::TaskKey("provide_plugin", pluginLocation.getPath().toString());
//         m_dependencies.insert(m_pluginTarget);
//         m_phase = Phase::PROVIDE_PLUGIN;
//     } else {
//         m_phase = Phase::COMPLETE;
//     }
//
//     return {};
// }
//
// tempo_utils::Status
// zuri_build::ProvideModuleTask::providePlugin(
//     const std::string &taskHash,
//     const absl::flat_hash_map<lyric_build::TaskKey,lyric_build::TaskState> &depStates,
//     lyric_build::BuildState *buildState)
// {
//     tempo_utils::UrlPath pluginArtifactPath;
//     TU_ASSIGN_OR_RETURN (pluginArtifactPath, lyric_build::convert_module_location_to_artifact_path(
//         m_moduleLocation, absl::StrCat(
//             ".", tempo_utils::sharedLibraryPlatformId(), tempo_utils::sharedLibraryFileDotSuffix())));
//
//     auto artifactCache = buildState->getArtifactCache();
//
//     // pull forward the plugin from the completed target
//     TU_ASSERT (depStates.contains(m_pluginTarget));
//     const auto &taskState = depStates.at(m_pluginTarget);
//
//     // if the target state is not completed, then fail the task
//     if (taskState.getStatus() != lyric_build::TaskState::Status::COMPLETED)
//         return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kTaskFailure,
//             "dependent task {} did not complete", m_pluginTarget.toString());
//
//     auto hash = taskState.getHash();
//     if (hash.empty())
//         return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kBuildInvariant,
//             "dependent task {} has invalid hash", m_pluginTarget.toString());
//     lyric_build::TraceId artifactTrace(hash, m_pluginTarget.getDomain(), m_pluginTarget.getId());
//     tempo_utils::UUID generation;
//     TU_ASSIGN_OR_RETURN (generation, artifactCache->loadTrace(artifactTrace));
//
//     // TODO: support task setting to specify the source artifact path explicitly
//     lyric_build::ArtifactId pluginSrcId(generation, hash, pluginArtifactPath);
//     lyric_build::ArtifactId pluginDstId(getGeneration(), taskHash, pluginArtifactPath);
//
//     // serialize the override metadata
//     lyric_build::MetadataWriter pluginWriter;
//     TU_RETURN_IF_NOT_OK (pluginWriter.configure());
//     pluginWriter.putAttr(lyric_build::kLyricBuildContentType, std::string(lyric_common::kPluginContentType));
//     pluginWriter.putAttr(lyric_build::kLyricBuildModuleLocation, m_moduleLocation);
//     lyric_build::LyricMetadata pluginMetadata;
//     TU_ASSIGN_OR_RETURN (pluginMetadata, pluginWriter.toMetadata());
//
//     TU_RETURN_IF_NOT_OK (artifactCache->linkArtifactOverridingMetadata(pluginDstId, pluginMetadata, pluginSrcId));
//
//     logInfo("linked plugin at {}", pluginDstId.toString());
//
//     m_phase = Phase::COMPLETE;
//     return {};
// }

tempo_utils::Status
zuri_build::ProvideModuleTask::runTask(lyric_build::TempDirectory *tempDirectory)
{
    tempo_utils::UrlPath objectArtifactPath;
    TU_ASSIGN_OR_RETURN (objectArtifactPath, lyric_build::convert_module_location_to_artifact_path(
        m_moduleLocation, lyric_common::kObjectFileDotSuffix));

    // serialize the override metadata
    lyric_build::MetadataWriter objectWriter;
    TU_RETURN_IF_NOT_OK (objectWriter.configure());
    objectWriter.putAttr(lyric_build::kLyricBuildContentType, std::string(lyric_common::kObjectContentType));
    objectWriter.putAttr(lyric_build::kLyricBuildModuleLocation, m_moduleLocation);
    lyric_build::LyricMetadata objectMetadata;
    TU_ASSIGN_OR_RETURN (objectMetadata, objectWriter.toMetadata());

    if (m_objectTarget.isValid()) {
        // if object target was specified then pull forward the object from the completed target
        TU_RETURN_IF_NOT_OK (linkArtifactOverridingMetadata(m_objectTarget, objectArtifactPath, objectMetadata));
        logInfo("linked object {}", objectArtifactPath.toString());
    } else {
        // otherwise store the existing object
        TU_RETURN_IF_NOT_OK (storeArtifact(objectArtifactPath, m_existingContent, objectMetadata));
        logInfo("stored object {}", objectArtifactPath.toString());
    }

    if (m_pluginTarget.isValid()) {
        tempo_utils::UrlPath pluginArtifactPath;
        TU_ASSIGN_OR_RETURN (pluginArtifactPath, lyric_build::convert_module_location_to_artifact_path(
            m_pluginLocation, absl::StrCat(
                ".", tempo_utils::sharedLibraryPlatformId(), tempo_utils::sharedLibraryFileDotSuffix())));

        // serialize the override metadata
        lyric_build::MetadataWriter pluginWriter;
        TU_RETURN_IF_NOT_OK (pluginWriter.configure());
        pluginWriter.putAttr(lyric_build::kLyricBuildContentType, std::string(lyric_common::kPluginContentType));
        pluginWriter.putAttr(lyric_build::kLyricBuildModuleLocation, m_pluginLocation);
        lyric_build::LyricMetadata pluginMetadata;
        TU_ASSIGN_OR_RETURN (pluginMetadata, pluginWriter.toMetadata());

        // if plugin target was specified then pull forward the plugin from the completed target
        TU_RETURN_IF_NOT_OK (linkArtifactOverridingMetadata(m_pluginTarget, pluginArtifactPath, pluginMetadata));
        logInfo("linked plugin {}", pluginArtifactPath.toString());
    }

    return {};
}

lyric_build::BaseTask *
zuri_build::new_provide_module_task(
    const lyric_build::BuildGeneration &generation,
    const lyric_build::TaskKey &key,
    std::weak_ptr<lyric_build::BuildState> buildState,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new ProvideModuleTask(generation, key, std::move(buildState), std::move(span));
}
