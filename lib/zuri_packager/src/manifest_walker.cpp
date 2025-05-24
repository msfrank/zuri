
#include <zuri_packager/internal/manifest_reader.h>
#include <zuri_packager/manifest_walker.h>

zuri_packager::ManifestWalker::ManifestWalker()
{
}

zuri_packager::ManifestWalker::ManifestWalker(std::shared_ptr<const internal::ManifestReader> reader)
    : m_reader(reader)
{
}

zuri_packager::ManifestWalker::ManifestWalker(const ManifestWalker &other)
    : m_reader(other.m_reader)
{
}

bool
zuri_packager::ManifestWalker::isValid() const
{
    return m_reader && m_reader->isValid();
}

zuri_packager::EntryWalker
zuri_packager::ManifestWalker::getRoot() const
{
    return getEntry(tempo_utils::UrlPath::fromString("/"));
}

bool
zuri_packager::ManifestWalker::hasEntry(const tempo_utils::UrlPath &entryPath) const
{
    auto *pathDescriptor = m_reader->findPath(entryPath.pathView());
    if (pathDescriptor == nullptr)
        return false;
    auto *entryDescriptor = m_reader->getEntry(pathDescriptor->entry());
    return entryDescriptor != nullptr;
}

zuri_packager::EntryWalker
zuri_packager::ManifestWalker::getEntry(tu_uint32 offset) const
{
    return EntryWalker(m_reader, offset);
}

zuri_packager::EntryWalker
zuri_packager::ManifestWalker::getEntry(const tempo_utils::UrlPath &entryPath) const
{
    auto *pathDescriptor = m_reader->findPath(entryPath.pathView());
    if (pathDescriptor == nullptr)
        return {};
    return EntryWalker(m_reader, pathDescriptor->entry());
}

tu_uint32
zuri_packager::ManifestWalker::numEntries() const
{
    return m_reader->numEntries();
}
