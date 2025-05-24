
#include <zuri_packager/entry_path.h>
#include <tempo_utils/log_stream.h>

zuri_packager::EntryPath::EntryPath()
{
}

zuri_packager::EntryPath::EntryPath(const tempo_utils::UrlPath &path)
    : m_path(path)
{
}

zuri_packager::EntryPath::EntryPath(const zuri_packager::EntryPath &other)
    : m_path(other.m_path)
{
}

bool
zuri_packager::EntryPath::isValid() const
{
    return m_path.isValid();
}

bool
zuri_packager::EntryPath::isEmpty() const
{
    return m_path.isEmpty();
}

int
zuri_packager::EntryPath::numParts() const
{
    return m_path.numParts();
}

std::string
zuri_packager::EntryPath::getPart(int index) const
{
    return m_path.getPart(index).getPart();
}

std::string_view
zuri_packager::EntryPath::partView(int index) const
{
    return m_path.partView(index);
}

zuri_packager::EntryPath
zuri_packager::EntryPath::getInit() const
{
    return EntryPath(m_path.getInit());
}

zuri_packager::EntryPath
zuri_packager::EntryPath::getTail() const
{
    return EntryPath(m_path.getTail());
}

std::string
zuri_packager::EntryPath::getFilename() const
{
    return m_path.getLast().getPart();
}

std::string_view
zuri_packager::EntryPath::filenameView() const
{
    return m_path.lastView();
}

std::string_view
zuri_packager::EntryPath::pathView() const
{
    return m_path.pathView();
}

zuri_packager::EntryPath
zuri_packager::EntryPath::traverse(std::string_view part)
{
    auto path = m_path.traverse(tempo_utils::UrlPathPart(part));
    return EntryPath(path);
}

std::string
zuri_packager::EntryPath::toString() const
{
    return m_path.toString();
}

tempo_utils::Url
zuri_packager::EntryPath::toUrl() const
{
    return tempo_utils::Url::fromRelative(m_path.pathView());
}

bool
zuri_packager::EntryPath::operator==(const zuri_packager::EntryPath &other) const
{
    return m_path == other.m_path;
}

bool
zuri_packager::EntryPath::operator!=(const zuri_packager::EntryPath &other) const
{
    return !(*this == other);
}

zuri_packager::EntryPath
zuri_packager::EntryPath::fromString(std::string_view s)
{
    return EntryPath(tempo_utils::UrlPath::fromString(s));
}

tempo_utils::LogMessage&&
zuri_packager::operator<<(tempo_utils::LogMessage &&message, const zuri_packager::EntryPath &entryPath)
{
    std::forward<tempo_utils::LogMessage>(message) << entryPath.toString();
    return std::move(message);
}