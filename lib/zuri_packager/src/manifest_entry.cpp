
#include <zuri_packager/manifest_attr.h>
#include <zuri_packager/manifest_entry.h>

zuri_packager::ManifestEntry::ManifestEntry(
    EntryType type,
    const tempo_utils::UrlPath &path,
    EntryAddress address,
    ManifestState *state)
    : m_type(type),
      m_path(path),
      m_address(address),
      m_offset(kInvalidOffsetU32),
      m_size(kInvalidOffsetU32),
      m_state(state)
{
    TU_ASSERT (m_address.isValid());
    TU_ASSERT (m_state != nullptr);
}

zuri_packager::EntryType
zuri_packager::ManifestEntry::getEntryType() const
{
    return m_type;
}

tempo_utils::UrlPath
zuri_packager::ManifestEntry::getEntryPath() const
{
    return m_path;
}

std::string
zuri_packager::ManifestEntry::getEntryName() const
{
    if (m_path.isValid())
        return m_path.getLast().getPart();
    return {};
}

zuri_packager::EntryAddress
zuri_packager::ManifestEntry::getAddress() const
{
    return m_address;
}

tu_uint32
zuri_packager::ManifestEntry::getEntryOffset() const
{
    return m_offset;
}

void
zuri_packager::ManifestEntry::setEntryOffset(tu_uint32 offset)
{
    m_offset = offset;
}

tu_uint32
zuri_packager::ManifestEntry::getEntrySize() const
{
    return m_size;
}

void
zuri_packager::ManifestEntry::setEntrySize(tu_uint32 size)
{
    m_size = size;
}

zuri_packager::EntryAddress
zuri_packager::ManifestEntry::getEntryDict() const
{
    return m_dict;
}

void
zuri_packager::ManifestEntry::setEntryDict(EntryAddress dict)
{
    m_dict = dict;
}

zuri_packager::EntryAddress
zuri_packager::ManifestEntry::getEntryLink() const
{
    return m_link;
}

void
zuri_packager::ManifestEntry::setEntryLink(EntryAddress link)
{
    m_link = link;
}

bool
zuri_packager::ManifestEntry::hasAttr(const AttrId &attrId) const
{
    return m_attrs.contains(attrId);
}

zuri_packager::AttrAddress
zuri_packager::ManifestEntry::getAttr(const AttrId &attrId) const
{
    if (m_attrs.contains(attrId))
        return m_attrs.at(attrId);
    return {};
}

zuri_packager::PackageStatus
zuri_packager::ManifestEntry::putAttr(ManifestAttr *attr)
{
    TU_ASSERT (attr != nullptr);

    auto attrId = attr->getAttrId();
    if (m_attrs.contains(attrId)) {
        return PackageStatus::forCondition(
            PackageCondition::kPackageInvariant, "entry contains duplicate attr");
    }
    m_attrs[attrId] = attr->getAddress();
    return {};
}

absl::flat_hash_map<
    zuri_packager::AttrId,
    zuri_packager::AttrAddress>::const_iterator
zuri_packager::ManifestEntry::attrsBegin() const
{
    return m_attrs.cbegin();
}

absl::flat_hash_map<
    zuri_packager::AttrId,
    zuri_packager::AttrAddress>::const_iterator
zuri_packager::ManifestEntry::attrsEnd() const
{
    return m_attrs.cend();
}

int
zuri_packager::ManifestEntry::numAttrs() const
{
    return m_attrs.size();
}

bool
zuri_packager::ManifestEntry::hasChild(std::string_view name) const
{
    return m_children.contains(name);
}

zuri_packager::EntryAddress
zuri_packager::ManifestEntry::getChild(std::string_view name)
{
    if (m_children.contains(name))
        return m_children.at(name);
    return {};
}

zuri_packager::PackageStatus
zuri_packager::ManifestEntry::putChild(ManifestEntry *child)
{
    TU_ASSERT (child != nullptr);
    auto name = child->getEntryName();
    if (name.empty())
        return PackageStatus::forCondition(
            PackageCondition::kPackageInvariant, "invalid entry name");
    m_children[name] = child->getAddress();
    return {};
}

absl::flat_hash_map<
    std::string,
    zuri_packager::EntryAddress>::const_iterator
zuri_packager::ManifestEntry::childrenBegin() const
{
    return m_children.cbegin();
}

absl::flat_hash_map<
    std::string,
    zuri_packager::EntryAddress>::const_iterator
zuri_packager::ManifestEntry::childrenEnd() const
{
    return m_children.cend();
}

int zuri_packager::ManifestEntry::numChildren() const
{
    return m_children.size();
}
