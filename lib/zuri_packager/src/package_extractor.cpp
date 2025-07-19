
#include <lyric_build/build_types.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_utils/directory_maker.h>
#include <tempo_utils/file_writer.h>
#include <tempo_utils/tempdir_maker.h>
#include <zuri_packager/package_extractor.h>
#include <zuri_packager/package_reader.h>
#include <zuri_packager/package_specifier.h>

zuri_packager::PackageExtractor::PackageExtractor(
    std::shared_ptr<PackageReader> reader,
    const PackageExtractorOptions &options)
    : m_reader(std::move(reader)),
      m_options(options)
{
    TU_ASSERT (m_reader != nullptr);
}

tempo_utils::Status
zuri_packager::PackageExtractor::configure()
{
    if (m_specifier.isValid())
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "extractor is already configured");

    TU_ASSIGN_OR_RETURN (m_specifier, m_reader->readPackageSpecifier());

    return {};
}

tempo_utils::Status
zuri_packager::PackageExtractor::extractRoot(const EntryWalker &root)
{
    if (root.getEntryType() != EntryType::Package)
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "invalid root entry");

    m_pendingDirectories.push(root);

    while (!m_pendingDirectories.empty()) {
        auto curr = m_pendingDirectories.front();
        m_pendingDirectories.pop();
        TU_RETURN_IF_NOT_OK (extractChildren(curr));
    }

    // resolve links
    while (!m_unresolvedLinks.empty()) {
        TU_RETURN_IF_NOT_OK (linkEntry(m_unresolvedLinks.front()));
        m_unresolvedLinks.pop();
    }

    return {};
}

tempo_utils::Status
zuri_packager::PackageExtractor::extractChildren(const EntryWalker &parent)
{
    // make directory
    auto relativePath = parent.getPath().toRelative();
    auto directoryPath = relativePath.toFilesystemPath(m_workdirPath);
    tempo_utils::DirectoryMaker directoryMaker(directoryPath);
    TU_RETURN_IF_NOT_OK (directoryMaker.getStatus());

    // process each child
    for (int i = 0; i < parent.numChildren(); i++) {
        auto child = parent.getChild(i);
        switch (child.getEntryType()) {
            case EntryType::File: {
                TU_RETURN_IF_NOT_OK (extractFile(child));
                break;
            }
            case EntryType::Directory: {
                m_pendingDirectories.push(child);
                break;
            }
            case EntryType::Link: {
                return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
                    "link entry type is unsupported");
            }
            default:
                return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
                    "invalid entry type");
        }
    }
    return {};
}

tempo_utils::Status
zuri_packager::PackageExtractor::extractFile(const EntryWalker &file)
{
    auto relativePath = file.getPath().toRelative();
    auto filePath = relativePath.toFilesystemPath(m_workdirPath);
    tempo_utils::Slice slice;
    TU_ASSIGN_OR_RETURN (slice, m_reader->readFileContents(file.getPath()));
    tempo_utils::FileWriter fileWriter(filePath, slice.toImmutableBytes(), tempo_utils::FileWriterMode::CREATE_ONLY);
    return fileWriter.getStatus();
}

tempo_utils::Status
zuri_packager::PackageExtractor::linkEntry(const EntryWalker &link)
{
    return {};
}

tempo_utils::Result<std::filesystem::path>
zuri_packager::PackageExtractor::extractPackage()
{
    if (!m_specifier.isValid())
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "extractor is not configured");

    //
    auto templateName = m_specifier.toDirectoryPath();
    templateName.replace_extension(".XXXXXXXX");
    auto workingRoot = !m_options.workingRoot.empty()? m_options.workingRoot : std::filesystem::current_path();

    //
    tempo_utils::TempdirMaker workdirMaker(workingRoot, templateName.string());
    TU_RETURN_IF_NOT_OK (workdirMaker.getStatus());
    m_workdirPath = workdirMaker.getTempdir();

    auto manifest = m_reader->getManifest();
    auto manifestRoot = manifest.getManifest();
    auto rootEntry = manifestRoot.getEntry(tempo_utils::UrlPath::fromString("/"));
    TU_RETURN_IF_NOT_OK (extractRoot(rootEntry));

    auto destinationRoot = !m_options.destinationRoot.empty()? m_options.destinationRoot : std::filesystem::current_path();
    auto destinationPath = m_specifier.toDirectoryPath(destinationRoot);
    std::filesystem::rename(m_workdirPath, destinationPath);

    return destinationPath;
}