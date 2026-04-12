#ifndef ZURI_BUILD_BASE_PROVIDER_TASK_H
#define ZURI_BUILD_BASE_PROVIDER_TASK_H

#include <lyric_assembler/object_state.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_hasher.h>

namespace zuri_build {

    struct ExistingArtifact {
        enum class Type {
            Invalid,
            File,
            ExternalFile,
            Url,
        };

        struct FileConfig {
            tempo_utils::UrlPath filePath;
        };
        struct ExternalFileConfig {
            std::filesystem::path externalFilePath;
        };
        struct UrlConfig {
            tempo_utils::Url url;
        };

        Type type = Type::Invalid;
        std::variant<std::nullptr_t,FileConfig,ExternalFileConfig,UrlConfig> config = nullptr;
    };

    class ProviderTaskOperations {
    public:
        virtual ~ProviderTaskOperations() = default;

        tempo_utils::Result<Option<lyric_build::Resource>> findProviderConfig(
            const tempo_utils::UrlPath &basePath,
            std::shared_ptr<lyric_build::AbstractVirtualFilesystem> &virtualFilesystem);

        tempo_utils::Result<lyric_build::TaskKey> configureProvider(
            const lyric_build::Resource &resource,
            std::shared_ptr<lyric_build::AbstractVirtualFilesystem> &virtualFilesystem);

        tempo_utils::Result<ExistingArtifact> configureExistingArtifact(
            const tempo_config::ConfigNode &existingNode);

        tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>> loadExistingArtifact(
            const ExistingArtifact &existing,
            std::shared_ptr<lyric_build::AbstractVirtualFilesystem> &virtualFilesystem);

        tempo_utils::Status hashExistingArtifact(
            const ExistingArtifact &existingArtifact,
            const std::shared_ptr<const tempo_utils::ImmutableBytes> &content,
            lyric_build::TaskHasher &hasher);
    };
}

#endif // ZURI_BUILD_BASE_PROVIDER_TASK_H
