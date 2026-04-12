
#include <lyric_build/base_task.h>
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_conversions.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/metadata_writer.h>
#include <lyric_build/task_hasher.h>
#include <lyric_build/task_utils.h>
#include <lyric_common/common_types.h>
#include <tempo_config/base_conversions.h>
#include <tempo_utils/file_reader.h>
#include <tempo_utils/log_message.h>
#include <zuri_build/provide_object_task.h>

zuri_build::ProvideObjectTask::ProvideObjectTask(
    const lyric_build::BuildGeneration &generation,
    const lyric_build::TaskKey &key,
    std::weak_ptr<lyric_build::BuildState> buildState,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, std::move(buildState), std::move(span))
{
}

tempo_utils::Status
zuri_build::ProvideObjectTask::configureTask(const lyric_build::TaskSettings &taskSettings)
{
    auto taskId = getId();
    auto settings = taskSettings.merge(lyric_build::TaskSettings({}, {}, {{taskId, getParams()}}));

    auto modulePath = tempo_utils::UrlPath::fromString(taskId.getId());
    if (!modulePath.isValid())
        return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kInvalidConfiguration,
            "task key id {} is not a valid object location", taskId.getId());

    m_objectLocation = lyric_common::ModuleLocation::fromString(modulePath.toString());

    // if task has a provider.config, then use it to configure the task
    if (m_objectLocation.isRelative()) {
        auto buildState = getBuildState();
        auto vfs = buildState->getVirtualFilesystem();

        Option<lyric_build::Resource> resourceOption;
        TU_ASSIGN_OR_RETURN (resourceOption, findProviderConfig(m_objectLocation.getPath(), vfs));

        if (resourceOption.hasValue()) {
            auto resource = resourceOption.getValue();
            TU_ASSIGN_OR_RETURN (m_provideTarget, configureProvider(resource, vfs));
            requestDependency(m_provideTarget);
            return {};
        }
    }

    // if 'existingObject' setting is defined then use it to configure the task
    auto taskSection = settings.getTaskSection(taskId);
    if (taskSection.mapContains("existingObject")) {
        auto existingNode = taskSection.mapAt("existingObject");
        TU_ASSIGN_OR_RETURN (m_existingObject, configureExistingArtifact(existingNode));
        return {};
    }

    // otherwise use the default provide target
    m_provideTarget = lyric_build::TaskKey("compile_object", taskId.getId());
    requestDependency(m_provideTarget);

    return {};
}

tempo_utils::Status
zuri_build::ProvideObjectTask::deduplicateTask(lyric_build::TaskHash &taskHash)
{
    lyric_build::TaskHasher hasher(getKey());
    hasher.hashValue(m_objectLocation.toString());

    if (m_existingObject.type != ExistingArtifact::Type::Invalid) {
        auto buildState = getBuildState();
        auto vfs = buildState->getVirtualFilesystem();
        TU_ASSIGN_OR_RETURN (m_existingContent, loadExistingArtifact(m_existingObject, vfs));
        TU_RETURN_IF_NOT_OK (hashExistingArtifact(m_existingObject, m_existingContent, hasher));
    } else {
        hasher.hashTask(this);
    }

    taskHash = hasher.finish();

    return {};
}

tempo_utils::Status
zuri_build::ProvideObjectTask::runTask(lyric_build::TempDirectory *tempDirectory)
{
    tempo_utils::UrlPath objectArtifactPath;
    TU_ASSIGN_OR_RETURN (objectArtifactPath, lyric_build::convert_module_location_to_artifact_path(
        m_objectLocation, lyric_common::kObjectFileDotSuffix));

    // serialize the override metadata
    lyric_build::MetadataWriter objectWriter;
    TU_RETURN_IF_NOT_OK (objectWriter.configure());
    objectWriter.putAttr(lyric_build::kLyricBuildContentType, std::string(lyric_common::kObjectContentType));
    objectWriter.putAttr(lyric_build::kLyricBuildModuleLocation, m_objectLocation);
    lyric_build::LyricMetadata objectMetadata;
    TU_ASSIGN_OR_RETURN (objectMetadata, objectWriter.toMetadata());

    if (m_provideTarget.isValid()) {
        // if provide target was specified then pull forward the object from the completed target
        TU_RETURN_IF_NOT_OK (linkArtifactOverridingMetadata(m_provideTarget, objectArtifactPath, objectMetadata));
        logInfo("linked object {}", objectArtifactPath.toString());
    } else {
        // otherwise store the existing object
        TU_RETURN_IF_NOT_OK (storeArtifact(objectArtifactPath, m_existingContent, objectMetadata));
        logInfo("stored object {}", objectArtifactPath.toString());
    }

    return {};
}

lyric_build::BaseTask *
zuri_build::new_provide_object_task(
    const lyric_build::BuildGeneration &generation,
    const lyric_build::TaskKey &key,
    std::weak_ptr<lyric_build::BuildState> buildState,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new ProvideObjectTask(generation, key, std::move(buildState), std::move(span));
}
