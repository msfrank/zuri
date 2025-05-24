
#include <lyric_build/build_diagnostics.h>
#include <lyric_build/build_result.h>
#include <tempo_utils/directory_maker.h>
#include <tempo_utils/file_writer.h>
#include <tempo_utils/log_message.h>
#include <zuri_build/target_writer.h>

TargetWriter::TargetWriter(
    const std::filesystem::path &installRoot,
    const zuri_packager::PackageSpecifier &specifier)
    : m_installRoot(installRoot),
      m_specifier(specifier)
{
    TU_ASSERT (!m_installRoot.empty());
    TU_ASSERT (m_specifier.isValid());
}

tempo_utils::Status
TargetWriter::configure()
{
    if (m_packagePath != nullptr)
        return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kBuildInvariant,
            "target writer is already configured");

    auto packageTempname = absl::StrCat(m_specifier.toString(), ".XXXXXXXX");
    auto packagePath = std::make_unique<tempo_utils::TempdirMaker>(m_installRoot, packageTempname);
    TU_RETURN_IF_NOT_OK (packagePath->getStatus());
    m_packagePath = std::move(packagePath);

    return {};
}

using perms = std::filesystem::perms;

tempo_utils::Status
TargetWriter::writeModule(
    const tempo_utils::UrlPath &filePath,
    const lyric_build::LyricMetadata &metadata,
    std::shared_ptr<const tempo_utils::ImmutableBytes> content)
{
    if (m_packagePath == nullptr)
        return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kBuildInvariant,
            "target writer is not configured");

    auto modulesRoot = m_packagePath->getTempdir() / "modules";
    auto absolutePath = filePath.toFilesystemPath(modulesRoot);

    auto parentPerms = perms::owner_all
        | perms::group_read
        | perms::group_exec
        | perms::others_read
        | perms::others_exec;

    auto parentPath = absolutePath.parent_path();
    tempo_utils::DirectoryMaker parentMaker(parentPath, parentPerms);
    TU_RETURN_IF_NOT_OK (parentMaker.getStatus());

    auto modulePerms = perms::owner_read
        | perms::owner_write
        | perms::group_read
        | perms::others_read;

    tempo_utils::FileWriter fileWriter(absolutePath, content,
        tempo_utils::FileWriterMode::CREATE_ONLY, modulePerms);
    TU_RETURN_IF_NOT_OK (fileWriter.getStatus());

    TU_LOG_INFO << "wrote module " << filePath << " to " << fileWriter.getAbsolutePath();

    return {};
}

// static tempo_utils::Status
// write_module_artifacts(
//     std::shared_ptr<lyric_build::AbstractCache> cache,
//     const tempo_utils::Url baseUrl,
//     const absl::btree_map<lyric_build::ArtifactId,lyric_build::LyricMetadata> &packageArtifacts,
//     zuri_packager::PackageWriter &packageWriter)
// {
//     // construct the object filter
//     lyric_build::MetadataWriter objectFilterWriter;
//     objectFilterWriter.putAttr(zuri_packager::kLyricPackagingContentType,
//         std::string(lyric_common::kObjectContentType));
//     auto toMetadataResult = objectFilterWriter.toMetadata();
//     if (toMetadataResult.isStatus())
//         return toMetadataResult.getStatus();
//     auto objectFilter = toMetadataResult.getResult();
//
//     auto baseOrigin = baseUrl.toOrigin();
//     auto basePath = baseUrl.toPath();
//
//     for (const auto &entry : packageArtifacts) {
//         // ignore artifacts which do not have the object content type
//         if (!lyric_build::metadata_matches_all_filters(entry.second, objectFilter))
//             continue;
//
//         const auto &artifactId = entry.first;
//         const auto metadata = entry.second.getMetadata();
//
//         // get the source url
//         tempo_utils::Url sourceUrl;
//         TU_RETURN_IF_NOT_OK (metadata.parseAttr(lyric_build::kLyricBuildContentUrl, sourceUrl));
//
//         if (sourceUrl.toOrigin() != baseOrigin || !sourceUrl.toPath().isDescendentOf(basePath))
//             return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kTaskFailure,
//                 "package artifact cannot be packaged; content url {} is not within the source base",
//                 sourceUrl.toString());
//
//         // get the module location
//         lyric_common::ModuleLocation moduleLocation;
//         TU_RETURN_IF_NOT_OK (metadata.parseAttr(lyric_build::kLyricBuildModuleLocation, moduleLocation));
//         auto locationUrl = moduleLocation.toUrl();
//
//         //
//         if (moduleLocation.hasScheme() || moduleLocation.hasAuthority())
//             return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kTaskFailure,
//                 "package artifact cannot be packaged; expected relative module location but found {}",
//                 moduleLocation.toString());
//
//         // construct the package entry path based on the module location path
//         auto modulePath = moduleLocation.getPath();
//         auto entryPath = zuri_packager::EntryPath::fromString("lib");
//         for (int i = 0; i < modulePath.numParts(); i++) {
//             entryPath = entryPath.traverse(modulePath.partView(i));
//         }
//
//         // make intermediate directories if necessary
//         zuri_packager::EntryPath directoryPath = entryPath.getInit();
//         zuri_packager::EntryAddress directoryAddress;
//         TU_ASSIGN_OR_RETURN (directoryAddress, packageWriter.makeDirectory(directoryPath, true));
//
//         // load the object content
//         std::shared_ptr<const tempo_utils::ImmutableBytes> content;
//         TU_ASSIGN_OR_RETURN (content, cache->loadContentFollowingLinks(artifactId));
//
//         // write the object content to the package
//         std::filesystem::path filename(entryPath.getFilename());
//         filename.replace_extension(lyric_common::kObjectFileDotSuffix);
//         TU_RETURN_IF_STATUS (packageWriter.putFile(directoryAddress, filename.string(), content));
//     }
//
//     return {};
// }
//
// tempo_utils::Status
// lyric_build::internal::PackageTask::package(
//     const std::string &taskHash,
//     const absl::flat_hash_map<TaskKey,TaskState> &depStates,
//     BuildState *buildState)
// {
//     auto cache = buildState->getCache();
//     auto span = getSpan();
//
//     // FIXME: not sure why flat hash map causes segfault but btree map doesn't
//     absl::btree_map<ArtifactId,LyricMetadata> packageArtifacts;
//
//     absl::flat_hash_map<std::string,uint32_t> directoryOffsets;
//
//     // determine the set of artifacts to write to the package
//     for (const auto &dep : depStates) {
//         const auto &taskKey = dep.first;
//         const auto &taskState = dep.second;
//
//         // if the target state is not completed, then fail the task
//         if (taskState.getStatus() != TaskState::Status::COMPLETED)
//             return BuildStatus::forCondition(BuildCondition::kTaskFailure,
//                 "dependent task {} did not complete", taskKey.toString());
//
//         auto hash = taskState.getHash();
//         if (hash.empty())
//             return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
//                 "dependent task {} has invalid hash", taskKey.toString());
//
//         TraceId artifactTrace(hash, taskKey.getDomain(), taskKey.getId());
//         auto generation = cache->loadTrace(artifactTrace);
//
//         std::vector<ArtifactId> targetArtifacts;
//         TU_ASSIGN_OR_RETURN (targetArtifacts, cache->findArtifacts(generation, hash, {}, {}));
//
//         for (const auto &artifactId : targetArtifacts) {
//             if (packageArtifacts.contains(artifactId))
//                 return BuildStatus::forCondition(BuildCondition::kTaskFailure,
//                     "duplicate artifact for {}", artifactId.toString());
//
//             LyricMetadata metadata;
//             TU_ASSIGN_OR_RETURN (metadata, cache->loadMetadataFollowingLinks(artifactId));
//             packageArtifacts[artifactId] = metadata;
//         }
//     }
//
//     zuri_packager::PackageWriterOptions options;
//     zuri_packager::PackageWriter packageWriter(options);
//     tempo_utils::Status status;
//
//     TU_RETURN_IF_NOT_OK (packageWriter.configure());
//
//     // construct the package
//     TU_RETURN_IF_NOT_OK (write_module_artifacts(cache, m_baseUrl, packageArtifacts, packageWriter));
//
//     // if specified, set main location attr on package entry
//     if (m_mainLocation.isValid()) {
//         TU_RETURN_IF_NOT_OK (packageWriter.putPackageAttr(
//             zuri_packager::kLyricPackagingMainLocation, m_mainLocation));
//     }
//
//     // serialize the package
//     std::shared_ptr<const tempo_utils::ImmutableBytes> packageBytes;
//     TU_ASSIGN_OR_RETURN (packageBytes, packageWriter.toBytes());
//
//     zuri_packager::PackageSpecifier specifier(m_packageName, m_packageDomain,
//         m_versionMajor, m_versionMinor, m_versionPatch);
//     auto packagePath = specifier.toFilesystemPath();
//     auto packageUrl = tempo_utils::Url::fromRelative(packagePath.string());
//
//     // store the object content in the build cache
//     ArtifactId packageArtifact(buildState->getGeneration().getUuid(), taskHash, packageUrl);
//     TU_RETURN_IF_NOT_OK (cache->storeContent(packageArtifact, packageBytes));
//
//     // serialize the object metadata
//     MetadataWriter writer;
//     writer.putAttr(kLyricBuildEntryType, EntryType::File);
//     writer.putAttr(zuri_packager::kLyricPackagingContentType, std::string(lyric_common::kPackageContentType));
//     writer.putAttr(zuri_packager::kLyricPackagingCreateTime, tempo_utils::millis_since_epoch());
//     LyricMetadata metadata;
//     TU_ASSIGN_OR_RETURN (metadata, writer.toMetadata());
//
//     // store the object metadata in the build cache
//     TU_RETURN_IF_NOT_OK (cache->storeMetadata(packageArtifact, metadata));
//
//     TU_LOG_V << "stored package " << packagePath << " at " << packageArtifact;
//
//     return {};
// }