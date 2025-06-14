
#include <tempo_config/config_builder.h>
#include <tempo_config/config_utils.h>
#include <tempo_utils/bytes_appender.h>
#include <tempo_utils/file_appender.h>
#include <zuri_packager/generated/manifest.h>
#include <zuri_packager/package_writer.h>

zuri_packager::PackageWriter::PackageWriter(
    const PackageSpecifier &specifier,
    const PackageWriterOptions &options)
    : m_specifier(specifier),
      m_options(options),
      m_packageEntry(nullptr)
{
    m_state = std::make_unique<ManifestState>();
    TU_ASSERT (m_specifier.isValid());
}

tempo_utils::Status
zuri_packager::PackageWriter::configure()
{
    if (m_state == nullptr)
        return PackageStatus::forCondition(
            PackageCondition::kPackageInvariant, "writer is finished");
    if (m_packageEntry != nullptr)
        return PackageStatus::forCondition(
            PackageCondition::kPackageInvariant, "writer is already configured");

    TU_ASSIGN_OR_RETURN (m_packageEntry, m_state->appendEntry(
        EntryType::Package, tempo_utils::UrlPath::fromString("/")));

    m_config = tempo_config::startMap()
        .put("name", tempo_config::valueNode(m_specifier.getPackageName()))
        .put("version", tempo_config::valueNode(m_specifier.getVersionString()))
        .put("domain", tempo_config::valueNode(m_specifier.getPackageDomain()))
    .buildMap();

    return {};
}

bool
zuri_packager::PackageWriter::hasEntry(const tempo_utils::UrlPath &path) const
{
    if (!path.isValid())
        return false;
    if (m_state == nullptr)
        return false;
    return m_state->hasEntry(path);
}

bool
zuri_packager::PackageWriter::hasEntry(EntryAddress parentDirectory, std::string_view name) const
{
    if (!parentDirectory.isValid())
        return false;
    if (m_state == nullptr)
        return false;
    auto *parent = m_state->getEntry(parentDirectory.getAddress());
    if (parent == nullptr)
        return false;
    return parent->hasChild(name);
}

zuri_packager::EntryAddress
zuri_packager::PackageWriter::getEntry(const tempo_utils::UrlPath &path) const
{
    if (!path.isValid())
        return {};
    if (m_state == nullptr)
        return {};
    auto *entry = m_state->getEntry(path);
    if (entry == nullptr)
        return {};
    return entry->getAddress();
}

tempo_utils::Result<zuri_packager::EntryAddress>
zuri_packager::PackageWriter::makeDirectory(const tempo_utils::UrlPath &path, bool createIntermediate)
{
    if (!path.isValid())
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "invalid path");
    if (m_state == nullptr)
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "writer is finished");
    if (m_packageEntry == nullptr)
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "writer is not configured");

    // if path is empty then return the package entry
    if (path.isEmpty()) {
        return m_packageEntry->getAddress();
    }

    // if we are not creating intermediate directories, then just try to create the directory
    // and return the result directly.
    if (!createIntermediate) {
        tempo_utils::UrlPath parentPath = path.getInit();
        return makeDirectory(getEntry(parentPath), path.getLast().partView());
    }

    auto address = m_packageEntry->getAddress();
    auto *entry = m_state->getEntry(address.getAddress());
    int i = 0;

    // skip over existing path entries
    while (i < path.numParts()) {
        auto part = path.partView(i);
        if (!entry->hasChild(part))
            break;
        address = entry->getChild(part);
        entry = m_state->getEntry(address.getAddress());
        if (entry->getEntryType() != EntryType::Directory)
            return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
                "existing path entry is not a directory");
        i++;
    }

    // if there are remaining path parts, then create a directory for each part
    while (i < path.numParts()) {
        auto part = path.partView(i);
        auto makeDirectoryResult = makeDirectory(address, part);
        if (makeDirectoryResult.isStatus())
            return makeDirectoryResult.getStatus();
        address = makeDirectoryResult.getResult();
        i++;
    }

    return address;
}

tempo_utils::Result<zuri_packager::EntryAddress>
zuri_packager::PackageWriter::makeDirectory(EntryAddress parentDirectory, std::string_view name)
{
    if (!parentDirectory.isValid())
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "invalid parent directory");
    if (name.empty())
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "invalid directory name");
    if (m_state == nullptr)
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "writer is finished");
    if (m_packageEntry == nullptr)
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "writer is not configured");

    auto *parentEntry = m_state->getEntry(parentDirectory.getAddress());
    if (parentEntry == nullptr)
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "missing parent entry");
    switch (parentEntry->getEntryType()) {
        case EntryType::Directory:
        case EntryType::Package:
            break;
        default:
            return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
                "invalid parent entry");
    }
    if (parentEntry->hasChild(name))
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "directory already contains entry");
    auto path = parentEntry->getEntryPath().traverse(tempo_utils::UrlPathPart(name));
    auto appendEntryResult = m_state->appendEntry(EntryType::Directory, path);
    if (appendEntryResult.isStatus())
        return appendEntryResult.getStatus();
    auto *entry = appendEntryResult.getResult();
    auto status = parentEntry->putChild(entry);
    if (status.notOk())
        return status;
    return entry->getAddress();
}

tempo_utils::Result<zuri_packager::EntryAddress>
zuri_packager::PackageWriter::putFile(
    EntryAddress parentDirectory,
    std::string_view name,
    std::shared_ptr<const tempo_utils::ImmutableBytes> bytes)
{
    if (!parentDirectory.isValid())
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "invalid parent directory");
    if (name.empty())
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "invalid file name");
    if (m_state == nullptr)
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "writer is finished");
    if (m_packageEntry == nullptr)
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "writer is not configured");

    auto *parentEntry = m_state->getEntry(parentDirectory.getAddress());
    if (parentEntry == nullptr)
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "missing parent directory");
    switch (parentEntry->getEntryType()) {
        case EntryType::Directory:
        case EntryType::Package:
            break;
        default:
            return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
                "invalid parent entry");
    }
    if (parentEntry->hasChild(name))
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "file already exists");

    auto path = parentEntry->getEntryPath().traverse(tempo_utils::UrlPathPart(name));
    auto appendEntryResult = m_state->appendEntry(EntryType::File, path);
    if (appendEntryResult.isStatus())
        return appendEntryResult.getStatus();
    auto *fileEntry = appendEntryResult.getResult();

    auto status = parentEntry->putChild(fileEntry);
    if (status.notOk())
        return status;

    m_contents[path] = bytes;
    return fileEntry->getAddress();
}

tempo_utils::Result<zuri_packager::EntryAddress>
zuri_packager::PackageWriter::putFile(
    const tempo_utils::UrlPath &path,
    std::shared_ptr<const tempo_utils::ImmutableBytes> bytes)
{
    if (!path.isValid())
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "invalid file path");
    if (m_state == nullptr)
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "writer is finished");
    if (m_packageEntry == nullptr)
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "writer is not configured");

    tempo_utils::UrlPath parentPath = path.getInit();
    auto name = path.getLast().getPart();
    auto *parentEntry = m_state->getEntry(parentPath);
    if (parentEntry == nullptr)
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "missing parent directory");
    switch (parentEntry->getEntryType()) {
        case EntryType::Directory:
        case EntryType::Package:
            break;
        default:
            return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
                "invalid parent entry");
    }
    if (parentEntry->hasChild(name))
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "file already exists");

    auto appendEntryResult = m_state->appendEntry(EntryType::File, path);
    if (appendEntryResult.isStatus())
        return appendEntryResult.getStatus();
    auto *entry = appendEntryResult.getResult();

    auto status = parentEntry->putChild(entry);
    if (status.notOk())
        return status;

    m_contents[path] = bytes;
    return entry->getAddress();
}

tempo_utils::Result<zuri_packager::EntryAddress>
zuri_packager::PackageWriter::linkToTarget(const tempo_utils::UrlPath &path, EntryAddress target)
{
    if (!path.isValid())
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "invalid link path");
    if (!target.isValid())
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "invalid link target");
    if (m_state == nullptr)
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "writer is finished");
    if (m_packageEntry == nullptr)
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "writer is not configured");

    if (hasEntry(path))
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "link already exists");

    // verify target exists
    auto *targetEntry = m_state->getEntry(target.getAddress());
    if (targetEntry == nullptr)
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "missing target entry");

    // verify that target doesn't exceed maximum link depth
    if (targetEntry->getEntryType() == EntryType::Link) {
        int currDepth = 1;
        while (targetEntry->getEntryType() == EntryType::Link) {
            if (currDepth > m_options.maxLinkDepth)
                return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
                    "exeeded maximum link depth");
            targetEntry = m_state->getEntry(targetEntry->getEntryLink().getAddress());
            if (targetEntry == nullptr)
                return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
                    "missing target entry");
        }
    }

    // only file and directory entries can be linked to
    switch (targetEntry->getEntryType()) {
        case EntryType::File:
        case EntryType::Directory:
            break;
        default:
            return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
                "invalid link target");
    }

    tempo_utils::UrlPath parentPath = path.getInit();
    auto name = path.getLast().getPart();
    auto *parentEntry = m_state->getEntry(parentPath);
    if (parentEntry == nullptr)
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "missing parent directory");
    if (parentEntry->getEntryType() != EntryType::Directory)
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "parent entry must be directory");
    if (parentEntry->hasChild(name))
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "entry already exists");

    auto appendEntryResult = m_state->appendEntry(EntryType::Link, path);
    if (appendEntryResult.isStatus())
        return appendEntryResult.getStatus();
    auto *linkEntry = appendEntryResult.getResult();
    linkEntry->setEntryLink(target);

    auto status = parentEntry->putChild(linkEntry);
    if (status.notOk())
        return status;
    return linkEntry->getAddress();
}

tempo_config::ConfigMap
zuri_packager::PackageWriter::getPackageConfig() const
{
    return m_config;
}

void
zuri_packager::PackageWriter::setPackageConfig(const tempo_config::ConfigMap &packageConfig)
{
    m_config = packageConfig;
}

tempo_utils::Result<std::filesystem::path>
zuri_packager::PackageWriter::writePackage()
{
    if (m_state == nullptr)
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "writer is finished");
    if (m_packageEntry == nullptr)
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "writer is not configured");

    //
    if (!m_options.skipPackageConfig) {
        std::string s;
        TU_RETURN_IF_NOT_OK (tempo_config::write_config_string(m_config, s));
        auto configContent = tempo_utils::MemoryBytes::copy(s);
        auto path = tempo_utils::UrlPath::fromString("/package.config");
        if (hasEntry(path))
            return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
                "/package.config already exists");
        TU_RETURN_IF_STATUS (putFile(path, configContent));
    }

    tempo_utils::FileAppenderMode mode = tempo_utils::FileAppenderMode::CREATE_ONLY;
    if (m_options.overwriteFile) {
        mode = tempo_utils::FileAppenderMode::CREATE_OR_OVERWRITE;
    }

    auto packagePath = m_specifier.toFilesystemPath(m_options.installRoot);
    tempo_utils::FileAppender appender(packagePath, mode);
    TU_RETURN_IF_NOT_OK (appender.getStatus());

    uint32_t currOffset = 0;

    // update the offset and size fields for all file entries
    for (int i = 0; i < m_state->numEntries(); i++) {
        auto *entry = m_state->getEntry(i);
        auto path = entry->getEntryPath();
        if (!m_contents.contains(path))
            continue;
        switch (entry->getEntryType()) {
            case EntryType::File:
                break;
            default:
                return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
                    "unexpected file content for entry");
        }
        auto bytes = m_contents.at(path);
        entry->setEntryOffset(currOffset);
        entry->setEntrySize(bytes->getSize());
        currOffset += bytes->getSize();
    }

    // serialize the manifest
    ZuriManifest manifest;
    TU_ASSIGN_OR_RETURN (manifest, m_state->toManifest());

    // write the manifest file identifier
    TU_RETURN_IF_NOT_OK (appender.appendBytes(
        std::string_view(zpk1::ManifestIdentifier(), strlen(zpk1::ManifestIdentifier()))));

    // write the prologue
    TU_RETURN_IF_NOT_OK (appender.appendU8(1));                         // version: u8
    TU_RETURN_IF_NOT_OK (appender.appendU8(0));                         // flags: u8

    // write the manifest
    auto manifestBytes = manifest.bytesView();
    TU_RETURN_IF_NOT_OK (appender.appendU32(manifestBytes.size()));     // manifestSize: u32
    TU_RETURN_IF_NOT_OK (appender.appendBytes(manifestBytes));          // manifest: bytes

    // write each entry contents
    for (int i = 0; i < m_state->numEntries(); i++) {
        const auto *entry = m_state->getEntry(i);
        if (entry->getEntryType() == EntryType::File) {
            auto path = entry->getEntryPath();
            if (!m_contents.contains(path))
                return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
                    "file entry has no contents");
            auto content = m_contents.at(path);
            std::span<const tu_uint8> bytes(content->getData(), content->getSize());
            TU_RETURN_IF_NOT_OK (appender.appendBytes(bytes));          // entry: bytes
        }
    }

    TU_RETURN_IF_NOT_OK (appender.finish());

    m_packageEntry = nullptr;
    m_state.reset();

    return packagePath;
}
