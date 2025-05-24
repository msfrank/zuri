
#include <zuri_packager/generated/manifest.h>
#include <zuri_packager/package_writer.h>
#include <tempo_utils/bytes_appender.h>
#include <tempo_utils/file_appender.h>

zuri_packager::PackageWriter::PackageWriter()
    : PackageWriter(PackageWriterOptions{})
{
}

zuri_packager::PackageWriter::PackageWriter(const PackageWriterOptions &options)
    : m_options(options),
      m_packageEntry(nullptr)
{
}

tempo_utils::Status
zuri_packager::PackageWriter::configure()
{
    if (m_packageEntry != nullptr)
        return PackageStatus::forCondition(
            PackageCondition::kPackageInvariant, "writer is already configured");
    auto appendEntryResult = m_state.appendEntry(EntryType::Package, EntryPath::fromString("/"));
    if (appendEntryResult.isStatus())
        return appendEntryResult.getStatus();
    m_packageEntry = appendEntryResult.getResult();
    return PackageStatus::ok();
}

bool
zuri_packager::PackageWriter::hasEntry(const EntryPath &path) const
{
    TU_ASSERT (path.isValid());
    return m_state.hasEntry(path);
}

bool
zuri_packager::PackageWriter::hasEntry(EntryAddress parentDirectory, std::string_view name) const
{
    if (!parentDirectory.isValid())
        return false;
    auto *parent = m_state.getEntry(parentDirectory.getAddress());
    if (parent == nullptr)
        return false;
    return parent->hasChild(name);
}

zuri_packager::EntryAddress
zuri_packager::PackageWriter::getEntry(const EntryPath &path) const
{
    TU_ASSERT (path.isValid());
    auto *entry = m_state.getEntry(path);
    if (entry == nullptr)
        return {};
    return entry->getAddress();
}

tempo_utils::Result<zuri_packager::EntryAddress>
zuri_packager::PackageWriter::makeDirectory(const EntryPath &path, bool createIntermediate)
{
    TU_ASSERT (path.isValid());

    // if path is empty then return the package entry
    if (path.isEmpty()) {
        return m_packageEntry->getAddress();
    }

    // if we are not creating intermediate directories, then just try to create the directory
    // and return the result directly.
    if (!createIntermediate) {
        EntryPath parentPath = path.getInit();
        return makeDirectory(getEntry(parentPath), path.getFilename());
    }

    auto address = m_packageEntry->getAddress();
    auto *entry = m_state.getEntry(address.getAddress());
    int i = 0;

    // skip over existing path entries
    while (i < path.numParts()) {
        auto part = path.partView(i);
        if (!entry->hasChild(part))
            break;
        address = entry->getChild(part);
        entry = m_state.getEntry(address.getAddress());
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
    auto *parentEntry = m_state.getEntry(parentDirectory.getAddress());
    if (parentEntry == nullptr)
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant, "missing parent entry");
    switch (parentEntry->getEntryType()) {
        case EntryType::Directory:
        case EntryType::Package:
            break;
        default:
            return PackageStatus::forCondition(PackageCondition::kPackageInvariant, "invalid parent entry");
    }
    if (parentEntry->hasChild(name))
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant, "directory already contains entry");
    EntryPath path = parentEntry->getEntryPath().traverse(name);
    auto appendEntryResult = m_state.appendEntry(EntryType::Directory, path);
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
    auto *parentEntry = m_state.getEntry(parentDirectory.getAddress());
    if (parentEntry == nullptr)
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant, "missing parent directory");
    switch (parentEntry->getEntryType()) {
        case EntryType::Directory:
        case EntryType::Package:
            break;
        default:
            return PackageStatus::forCondition(PackageCondition::kPackageInvariant, "invalid parent entry");
    }
    if (parentEntry->hasChild(name))
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant, "file already exists");

    EntryPath path = parentEntry->getEntryPath().traverse(name);
    auto appendEntryResult = m_state.appendEntry(EntryType::File, path);
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
    const EntryPath &path,
    std::shared_ptr<const tempo_utils::ImmutableBytes> bytes)
{
    TU_ASSERT (path.isValid());

    EntryPath parentPath = path.getInit();
    auto name = path.getFilename();
    auto *parentEntry = m_state.getEntry(parentPath);
    if (parentEntry == nullptr)
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant, "missing parent directory");
    switch (parentEntry->getEntryType()) {
        case EntryType::Directory:
        case EntryType::Package:
            break;
        default:
            return PackageStatus::forCondition(PackageCondition::kPackageInvariant, "invalid parent entry");
    }
    if (parentEntry->hasChild(name))
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant, "file already exists");

    auto appendEntryResult = m_state.appendEntry(EntryType::File, path);
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
zuri_packager::PackageWriter::linkToTarget(const EntryPath &path, EntryAddress target)
{
    TU_ASSERT (path.isValid());

    if (hasEntry(path))
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant, "link already exists");

    // verify target exists
    auto *targetEntry = m_state.getEntry(target.getAddress());
    if (targetEntry == nullptr)
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant, "missing target entry");

    // verify that target doesn't exceed maximum link depth
    if (targetEntry->getEntryType() == EntryType::Link) {
        int currDepth = 1;
        while (targetEntry->getEntryType() == EntryType::Link) {
            if (currDepth > m_options.maxLinkDepth)
                return PackageStatus::forCondition(PackageCondition::kPackageInvariant, "exeeded maximum link depth");
            targetEntry = m_state.getEntry(targetEntry->getEntryLink().getAddress());
            if (targetEntry == nullptr)
                return PackageStatus::forCondition(PackageCondition::kPackageInvariant, "missing target entry");
        }
    }

    // only file and directory entries can be linked to
    switch (targetEntry->getEntryType()) {
        case EntryType::File:
        case EntryType::Directory:
            break;
        default:
            return PackageStatus::forCondition(PackageCondition::kPackageInvariant, "invalid link target");
    }

    EntryPath parentPath = path.getInit();
    auto name = path.getFilename();
    auto *parentEntry = m_state.getEntry(parentPath);
    if (parentEntry == nullptr)
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant, "missing parent directory");
    if (parentEntry->getEntryType() != EntryType::Directory)
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant, "parent entry must be directory");
    if (parentEntry->hasChild(name))
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant, "entry already exists");

    auto appendEntryResult = m_state.appendEntry(EntryType::Link, path);
    if (appendEntryResult.isStatus())
        return appendEntryResult.getStatus();
    auto *linkEntry = appendEntryResult.getResult();
    linkEntry->setEntryLink(target);

    auto status = parentEntry->putChild(linkEntry);
    if (status.notOk())
        return status;
    return linkEntry->getAddress();
}

tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>>
zuri_packager::PackageWriter::toBytes() const
{
    uint32_t currOffset = 0;

    // update the offset and size fields for all file entries
    for (int i = 0; i < m_state.numEntries(); i++) {
        auto *entry = m_state.getEntry(i);
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
    auto toManifestResult = m_state.toManifest();
    if (toManifestResult.isStatus())
        return toManifestResult.getStatus();
    auto manifest = toManifestResult.getResult();

    tempo_utils::BytesAppender appender;

    // write the manifest file identifier
    appender.appendBytes(std::string_view(zpk1::ManifestIdentifier(), strlen(zpk1::ManifestIdentifier())));

    // write the prologue
    appender.appendU8(1);                                                   // version: u8
    appender.appendU8(0);                                                   // flags: u8

    // write the manifest
    auto manifestBytes = manifest.bytesView();
    appender.appendU32(manifestBytes.size());                               // manifestSize: u32
    appender.appendBytes(manifestBytes);                                    // manifest: bytes

    // write each entry contents
    for (int i = 0; i < m_state.numEntries(); i++) {
        const auto *entry = m_state.getEntry(i);
        if (entry->getEntryType() == EntryType::File) {
            auto path = entry->getEntryPath();
            if (!m_contents.contains(path))
                return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
                    "file entry has no contents");
            auto content = m_contents.at(path);
            std::span<const tu_uint8> bytes(content->getData(), content->getSize());

            appender.appendBytes(bytes);                                    // entry: bytes
        }
    }

    return static_pointer_cast<const tempo_utils::ImmutableBytes>(appender.finish());
}
