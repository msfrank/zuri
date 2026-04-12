
#include <lyric_build/build_conversions.h>
#include <lyric_build/build_result.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_hasher.h>
#include <lyric_build/task_utils.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_utils/file_reader.h>
#include <zuri_build/provider_task_operations.h>

#include "zuri_build/build_conversions.h"
#include "zuri_build/build_result.h"

tempo_utils::Result<Option<lyric_build::Resource>>
zuri_build::ProviderTaskOperations::findProviderConfig(
    const tempo_utils::UrlPath &basePath,
    std::shared_ptr<lyric_build::AbstractVirtualFilesystem> &virtualFilesystem)
{
    auto providerConfigPath = basePath.traverse(tempo_utils::UrlPathPart{"provider.config"});
    return virtualFilesystem->fetchResource(providerConfigPath);
}

tempo_utils::Result<lyric_build::TaskKey>
zuri_build::ProviderTaskOperations::configureProvider(
    const lyric_build::Resource &resource,
    std::shared_ptr<lyric_build::AbstractVirtualFilesystem> &virtualFilesystem)
{
    std::shared_ptr<const tempo_utils::ImmutableBytes> content;
    TU_ASSIGN_OR_RETURN (content, virtualFilesystem->loadResource(resource.id));

    tempo_config::ConfigMap providerMap;
    TU_ASSIGN_OR_RETURN (providerMap, tempo_config::read_config_map_string(content->getStringView()));

    lyric_build::TaskIdParser taskIdParser;
    lyric_build::TaskId taskId;
    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(taskId, taskIdParser, providerMap, "taskId"));

    tempo_config::ConfigMap taskSettings;
    if (providerMap.mapContains("taskSettings")) {
        auto settingsNode = providerMap.mapAt("taskSettings");
        if (settingsNode.getNodeType() != tempo_config::ConfigNodeType::kMap)
            return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kInvalidConfiguration,
                "invalid provider.config; 'taskSettings' must be a Map");
        taskSettings = settingsNode.toMap();
    }

    return lyric_build::TaskKey{taskId.getDomain(), taskId.getId(), taskSettings};
}

tempo_utils::Result<zuri_build::ExistingArtifact>
zuri_build::ProviderTaskOperations::configureExistingArtifact(const tempo_config::ConfigNode &existingNode)
{
    ExistingArtifactParser existingArtifactParser;
    ExistingArtifact existingArtifact;
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(existingArtifact, existingArtifactParser, existingNode));
    return existingArtifact;
}

tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>>
zuri_build::ProviderTaskOperations::loadExistingArtifact(
    const ExistingArtifact &existingArtifact,
    std::shared_ptr<lyric_build::AbstractVirtualFilesystem> &virtualFilesystem)
{
    switch (existingArtifact.type) {
        case ExistingArtifact::Type::File: {
            const auto &fileConfig = std::get<ExistingArtifact::FileConfig>(existingArtifact.config);
            Option<lyric_build::Resource> resourceOption;
            TU_ASSIGN_OR_RETURN (resourceOption, virtualFilesystem->fetchResource(fileConfig.filePath));
            if (resourceOption.isEmpty())
                return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kBuildInvariant,
                    "missing existing file {}", fileConfig.filePath.toString());
            auto &objectResource = resourceOption.peekValue();
            std::shared_ptr<const tempo_utils::ImmutableBytes> content;
            return virtualFilesystem->loadResource(objectResource.id);
        }
        case ExistingArtifact::Type::ExternalFile: {
            const auto &externalFileConfig = std::get<ExistingArtifact::ExternalFileConfig>(existingArtifact.config);
            tempo_utils::FileReader reader(externalFileConfig.externalFilePath);
            TU_RETURN_IF_NOT_OK (reader.getStatus());
            return reader.getBytes();
        }
        default:
            return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kBuildInvariant,
                "invalid existing artifact type");
    }
}

tempo_utils::Status
zuri_build::ProviderTaskOperations::hashExistingArtifact(
    const ExistingArtifact &existingArtifact,
    const std::shared_ptr<const tempo_utils::ImmutableBytes> &content,
    lyric_build::TaskHasher &hasher)
{
    hasher.hashValue(static_cast<tu_int64>(existingArtifact.type));
    switch (existingArtifact.type) {
        case ExistingArtifact::Type::File: {
            auto fileConfig = std::get<ExistingArtifact::FileConfig>(existingArtifact.config);
            hasher.hashValue(fileConfig.filePath.toString());
            break;
        }
        case ExistingArtifact::Type::ExternalFile: {
            auto externalFileConfig = std::get<ExistingArtifact::ExternalFileConfig>(existingArtifact.config);
            hasher.hashValue(externalFileConfig.externalFilePath.string());
            break;
        }
        case ExistingArtifact::Type::Url: {
            auto urlConfig = std::get<ExistingArtifact::UrlConfig>(existingArtifact.config);
            hasher.hashValue(urlConfig.url.toString());
            break;
        }
        default:
            return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kBuildInvariant,
                "invalid existing artifact type");
    }

    hasher.hashValue(content->getSpan());

    return {};
}