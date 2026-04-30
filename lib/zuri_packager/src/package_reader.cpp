
#include <lyric_common/common_conversions.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_utils/bytes_iterator.h>
#include <tempo_utils/log_message.h>
#include <zuri_packager/internal/manifest_reader.h>
#include <zuri_packager/package_reader.h>
#include <zuri_packager/packager_result.h>
#include <zuri_packager/packaging_conversions.h>

zuri_packager::PackageReader::PackageReader(
    tu_uint8 version,
    tu_uint8 flags,
    ZuriManifest manifest,
    tempo_utils::Slice contents,
    Private)
    : m_version(version),
      m_flags(flags),
      m_manifest(manifest),
      m_contents(std::move(contents))
{
    TU_ASSERT (m_manifest.isValid());
}

bool
zuri_packager::PackageReader::isValid() const
{
    return m_manifest.isValid();
}

tu_uint8
zuri_packager::PackageReader::getVersion() const
{
    return m_version;
}

tu_uint8
zuri_packager::PackageReader::getFlags() const
{
    return m_flags;
}

zuri_packager::ZuriManifest
zuri_packager::PackageReader::getManifest() const
{
    return m_manifest;
}

tempo_utils::Result<zuri_packager::PackageSpecifier>
zuri_packager::PackageReader::readPackageSpecifier() const
{
    tempo_config::ConfigMap packageConfig;
    TU_ASSIGN_OR_RETURN (packageConfig, readPackageConfig());

    tempo_config::StringParser nameParser;
    std::string packageName;
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(packageName, nameParser, packageConfig, "name"));

    tempo_config::StringParser versionParser;
    std::string packageVersion;
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(packageVersion, versionParser, packageConfig, "version"));

    tempo_config::StringParser domainParser;
    std::string packageDomain;
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(packageDomain, domainParser, packageConfig, "domain"));

    return PackageSpecifier::fromString(absl::StrCat(
        packageName, "-", packageVersion, "@", packageDomain));
}

tempo_utils::Result<lyric_common::ModuleLocation>
zuri_packager::PackageReader::readProgramMain() const
{
    tempo_config::ConfigMap packageConfig;
    TU_ASSIGN_OR_RETURN (packageConfig, readPackageConfig());

    lyric_common::ModuleLocationParser programMainParser;
    lyric_common::ModuleLocation programMain;
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(programMain, programMainParser,
        packageConfig, "programMain"));

    return programMain;
}

tempo_utils::Result<zuri_packager::RequirementsMap>
zuri_packager::PackageReader::readRequirementsMap() const
{
    tempo_config::ConfigMap packageConfig;
    TU_ASSIGN_OR_RETURN (packageConfig, readPackageConfig());

    RequirementsMapParser requirementsMapParser(RequirementsMap{});
    RequirementsMap requirementsMap;
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(requirementsMap, requirementsMapParser,
        packageConfig, "requirements"));

    return requirementsMap;
}

tempo_utils::Result<tempo_config::ConfigMap>
zuri_packager::PackageReader::readPackageConfig() const
{
    tempo_utils::Slice slice;
    TU_ASSIGN_OR_RETURN (slice, readFileContents(tempo_utils::UrlPath::fromString("/package.config")));

    //
    std::string_view packageConfigString((const char *) slice.getData(), slice.getSize());
    tempo_config::ConfigNode packageConfig;
    TU_ASSIGN_OR_RETURN (packageConfig, tempo_config::read_config_string(
        packageConfigString, std::make_shared<tempo_config::ConfigSource>(
            tempo_config::ConfigSourceType::File, "<zuri-package>/package.config")));

    return packageConfig.toMap();
}

static tempo_utils::Result<tempo_utils::Slice>
get_file_entry_contents(
    const zuri_packager::EntryWalker &walker,
    const tempo_utils::Slice &contents)
{
    if (walker.getEntryType() != zuri_packager::EntryType::File)
        return zuri_packager::PackagerStatus::forCondition(zuri_packager::PackagerCondition::kInvalidManifest,
            "invalid entry type");
    if (contents.getSize() < walker.getFileOffset())
        return zuri_packager::PackagerStatus::forCondition(zuri_packager::PackagerCondition::kInvalidManifest,
            "invalid file entry offset");
    if (contents.getSize() < walker.getFileOffset() + walker.getFileSize())
        return zuri_packager::PackagerStatus::forCondition(zuri_packager::PackagerCondition::kInvalidManifest,
            "invalid file entry size");
    return contents.slice(walker.getFileOffset(), walker.getFileSize());
}

tempo_utils::Result<tempo_utils::Slice>
zuri_packager::PackageReader::readFileContents(const tempo_utils::UrlPath &entryPath, bool followSymlinks) const
{
    if (!m_manifest.hasEntry(entryPath))
        return PackagerStatus::forCondition(PackagerCondition::kMissingEntry,
            "missing entry {}", entryPath.toString());

    auto entry = m_manifest.getEntry(entryPath);
    switch (entry.getEntryType()) {
        case EntryType::File:
            return get_file_entry_contents(entry, m_contents);
        case EntryType::Link:
            if (!followSymlinks)
                return PackagerStatus::forCondition(PackagerCondition::kInvalidManifest,
                    "invalid entry type");
            return get_file_entry_contents(entry.resolveLink(), m_contents);
        default:
            return PackagerStatus::forCondition(PackagerCondition::kInvalidManifest,
                "invalid entry type");
    }
}

// tempo_utils::Result<tu_uint32>
// zuri_packager::PackageReader::readFileSize(const tempo_utils::UrlPath &entryPath, bool followSymlinks) const
// {
//     auto manifest = m_manifest.getManifest();
//     auto entry = manifest.getEntry(entryPath);
//     if (!entry.isValid())
//         return 0;
//
//     switch (entry.getEntryType()) {
//         case EntryType::File:
//             return entry.getFileSize();
//         case EntryType::Link:
//             if (!followSymlinks)
//                 return 0;
//             entry = entry.resolveLink();
//             return entry.getEntryType() == EntryType::File? entry.getFileSize() : 0;
//         default:
//             return {};
//     }
// }

/**
 *
 * @param packageBytes
 * @return
 */
tempo_utils::Result<std::shared_ptr<zuri_packager::PackageReader>>
zuri_packager::PackageReader::create(std::shared_ptr<const tempo_utils::ImmutableBytes> packageBytes)
{
    tempo_utils::BytesIterator it(packageBytes->getSpan());

    // verify the package header
    if (it.bytesLeft() < 10)
        return PackagerStatus::forCondition(
            PackagerCondition::kInvalidHeader, "invalid header size");

    std::span<const tu_uint8> identifierSlice;
    it.nextSlice(identifierSlice, 4);

    if (strncmp((const char *) identifierSlice.data(), zpk1::ManifestIdentifier(), 4) != 0)
        return PackagerStatus::forCondition(
            PackagerCondition::kInvalidHeader, "invalid package identifier");

    // package version
    tu_uint8 version;
    it.nextU8(version);

    // package flags
    tu_uint8 flags;
    it.nextU8(flags);

    // manifest size
    tu_uint32 manifestSize;
    it.nextU32(manifestSize);

    auto manifestOffset = it.bytesConsumed();

    // verify the manifest
    if (it.bytesLeft() < manifestSize)
        return PackagerStatus::forCondition(
            PackagerCondition::kInvalidManifest, "invalid manifest size");

    std::span<const tu_uint8> manifestSlice;
    it.nextSlice(manifestSlice, manifestSize);

    auto contentsOffset = it.bytesConsumed();

    flatbuffers::Verifier verifier(manifestSlice.data(), manifestSlice.size());
    if (!zpk1::VerifyManifestBuffer(verifier))
        return PackagerStatus::forCondition(
            PackagerCondition::kInvalidManifest, "invalid package manifest");

    // allocate the manifest
    auto manifestBytes = tempo_utils::Slice(packageBytes, manifestOffset, manifestSize).toImmutableBytes();
    ZuriManifest manifest(manifestBytes);
    if (!manifest.isValid())
        return PackagerStatus::forCondition(
            PackagerCondition::kInvalidManifest, "invalid package manifest");

    // create a slice which spans only the package contents
    tempo_utils::Slice contents(packageBytes, contentsOffset, it.bytesLeft());

    return std::make_shared<PackageReader>(version, flags, manifest, contents, Private{});
}

/**
 *
 * @param packagePath
 * @return
 */
tempo_utils::Result<std::shared_ptr<zuri_packager::PackageReader>>
zuri_packager::PackageReader::open(const std::filesystem::path &packagePath)
{
    auto mmapFileResult = tempo_utils::MemoryMappedBytes::open(packagePath);
    if (mmapFileResult.isStatus())
        return mmapFileResult.getStatus();
    auto bytes = mmapFileResult.getResult();

    return create(static_pointer_cast<const tempo_utils::ImmutableBytes>(bytes));
}