
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
#include <tempo_utils/platform.h>
#include <zuri_build/provide_plugin_task.h>

zuri_build::ProvidePluginTask::ProvidePluginTask(
    const lyric_build::BuildGeneration &generation,
    const lyric_build::TaskKey &key,
    std::weak_ptr<lyric_build::BuildState> buildState,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, std::move(buildState), std::move(span))
{
}

tempo_utils::Status
zuri_build::ProvidePluginTask::configureTask(const lyric_build::TaskSettings &taskSettings)
{
    auto taskId = getId();
    auto settings = taskSettings.merge(lyric_build::TaskSettings({}, {}, {{taskId, getParams()}}));

    auto modulePath = tempo_utils::UrlPath::fromString(taskId.getId());
    if (!modulePath.isValid())
        return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kInvalidConfiguration,
            "task key id {} is not a valid plugin location", taskId.getId());

    m_pluginLocation = lyric_common::ModuleLocation::fromString(modulePath.toString());

    // if task has a provider.config, then use it to configure the task
    if (m_pluginLocation.isRelative()) {
        auto buildState = getBuildState();
        auto vfs = buildState->getVirtualFilesystem();

        Option<lyric_build::Resource> resourceOption;
        TU_ASSIGN_OR_RETURN (resourceOption, findProviderConfig(m_pluginLocation.getPath(), vfs));

        if (resourceOption.hasValue()) {
            auto resource = resourceOption.getValue();
            TU_ASSIGN_OR_RETURN (m_provideTarget, configureProvider(resource, vfs));
            requestDependency(m_provideTarget);
            return {};
        }
    }

    // if 'existingPlugin' setting is defined then use it to configure the task
    auto taskSection = settings.getTaskSection(taskId);
    if (taskSection.mapContains("existingPlugin")) {
        auto existingNode = taskSection.mapAt("existingPlugin");
        TU_ASSIGN_OR_RETURN (m_existingPlugin, configureExistingArtifact(existingNode));
        return {};
    }

    // otherwise use the default provide target
    m_provideTarget = lyric_build::TaskKey("compile_plugin", taskId.getId());
    requestDependency(m_provideTarget);

    return {};
}

tempo_utils::Status
zuri_build::ProvidePluginTask::deduplicateTask(lyric_build::TaskHash &taskHash)
{
    lyric_build::TaskHasher hasher(getKey());
    hasher.hashValue(m_pluginLocation.toString());

    if (m_existingPlugin.type != ExistingArtifact::Type::Invalid) {
        auto buildState = getBuildState();
        auto vfs = buildState->getVirtualFilesystem();
        TU_ASSIGN_OR_RETURN (m_existingContent, loadExistingArtifact(m_existingPlugin, vfs));
        TU_RETURN_IF_NOT_OK (hashExistingArtifact(m_existingPlugin, m_existingContent, hasher));
    } else {
        hasher.hashTask(this);
    }

    taskHash = hasher.finish();

    return {};
}

tempo_utils::Status
zuri_build::ProvidePluginTask::runTask(lyric_build::TempDirectory *tempDirectory)
{
    tempo_utils::UrlPath pluginArtifactPath;
    TU_ASSIGN_OR_RETURN (pluginArtifactPath, lyric_build::convert_module_location_to_artifact_path(
        m_pluginLocation, absl::StrCat(
            ".", tempo_utils::sharedLibraryPlatformId(), tempo_utils::sharedLibraryFileDotSuffix())));

    // construct the plugin metadata
    lyric_build::MetadataWriter pluginMetadataWriter;
    TU_RETURN_IF_NOT_OK (pluginMetadataWriter.configure());
    pluginMetadataWriter.putAttr(lyric_build::kLyricBuildContentType, std::string(lyric_common::kPluginContentType));
    pluginMetadataWriter.putAttr(lyric_build::kLyricBuildModuleLocation, m_pluginLocation);
    lyric_build::LyricMetadata pluginMetadata;
    TU_ASSIGN_OR_RETURN (pluginMetadata, pluginMetadataWriter.toMetadata());

    if (m_provideTarget.isValid()) {
        // if provide target was specified then pull forward the plugin from the completed target
        TU_RETURN_IF_NOT_OK (linkArtifactOverridingMetadata(m_provideTarget, pluginArtifactPath, pluginMetadata));
        logInfo("linked plugin {}", pluginArtifactPath.toString());
    } else {
        // otherwise store the existing plugin
        TU_RETURN_IF_NOT_OK (storeArtifact(pluginArtifactPath, m_existingContent, pluginMetadata));
        logInfo("stored plugin {}", pluginArtifactPath.toString());
    }

    return {};
}

lyric_build::BaseTask *
zuri_build::new_provide_plugin_task(
    const lyric_build::BuildGeneration &generation,
    const lyric_build::TaskKey &key,
    std::weak_ptr<lyric_build::BuildState> buildState,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new ProvidePluginTask(generation, key, std::move(buildState), std::move(span));
}
