
#include "zuri_packager/package_types.h"

#include <absl/strings/str_cat.h>
#include <absl/strings/str_split.h>
#include <zuri_packager/package_types.h>
#include <tempo_utils/log_message.h>
#include <tempo_utils/url_authority.h>

zuri_packager::AttrId::AttrId()
    : m_address(),
      m_type(kInvalidOffsetU32)
{
}

zuri_packager::AttrId::AttrId(const NamespaceAddress &address, tu_uint32 type)
    : m_address(address),
      m_type(type)
{
    TU_ASSERT (m_address.isValid());
    TU_ASSERT (m_type != kInvalidOffsetU32);
}

zuri_packager::AttrId::AttrId(const AttrId &other)
    : m_address(other.m_address),
      m_type(other.m_type)
{
}

zuri_packager::NamespaceAddress
zuri_packager::AttrId::getAddress() const
{
    return m_address;
}

tu_uint32
zuri_packager::AttrId::getType() const
{
    return m_type;
}

bool
zuri_packager::AttrId::operator==(const AttrId &other) const
{
    return m_address == other.m_address && m_type == other.m_type;
}

zuri_packager::PackageId::PackageId()
{
}

zuri_packager::PackageId::PackageId(std::string_view name, std::string_view domain)
    : m_priv(std::make_shared<Priv>())
{
    m_priv->name = name;
    m_priv->domain = domain;
    TU_ASSERT (!m_priv->name.empty());
    TU_ASSERT (!m_priv->domain.empty());
}

zuri_packager::PackageId::PackageId(const PackageId &other)
    : m_priv(other.m_priv)
{
}

bool
zuri_packager::PackageId::isValid() const
{
    return m_priv != nullptr;
}

std::string
zuri_packager::PackageId::getName() const
{
    if (isValid())
        return m_priv->name;
    return {};
}

std::string
zuri_packager::PackageId::getDomain() const
{
    if (isValid())
        return m_priv->domain;
    return {};
}

std::string
zuri_packager::PackageId::toString() const
{
    if (isValid())
        return absl::StrCat(m_priv->name, "@", m_priv->domain);
    return {};
}

int
zuri_packager::PackageId::compare(const PackageId &other) const
{
    if (m_priv != nullptr) {
        if (other.m_priv == nullptr)
            return 1;
        int namecmp = m_priv->name.compare(other.m_priv->name);
        if (namecmp != 0)
            return namecmp;
        return m_priv->domain.compare(other.m_priv->domain);
    }
    return other.m_priv == nullptr? 0 : -1;
}

bool
zuri_packager::PackageId::operator==(const PackageId &other) const
{
    return compare(other) == 0;
}

bool
zuri_packager::PackageId::operator!=(const PackageId &other) const
{
    return compare(other) != 0;
}

bool
zuri_packager::PackageId::operator<=(const PackageId &other) const
{
    return compare(other) <= 0;
}

bool
zuri_packager::PackageId::operator<(const PackageId &other) const
{
    return compare(other) < 0;
}

bool
zuri_packager::PackageId::operator>=(const PackageId &other) const
{
    return compare(other) >= 0;
}

bool
zuri_packager::PackageId::operator>(const PackageId &other) const
{
    return compare(other) > 0;
}

zuri_packager::PackageId
zuri_packager::PackageId::fromString(std::string_view s)
{
    auto authority = tempo_utils::UrlAuthority::fromString(s);
    return fromAuthority(authority);
}

zuri_packager::PackageId
zuri_packager::PackageId::fromAuthority(const tempo_utils::UrlAuthority &authority)
{
    if (!authority.isValid())
        return {};
    auto name = authority.getUsername();
    if (name.empty())
        return {};
    auto domain = authority.getHost();
    if (domain.empty())
        return {};
    return PackageId(name, domain);
}

zuri_packager::PackageVersion::PackageVersion()
{
}

zuri_packager::PackageVersion::PackageVersion(tu_uint32 majorVersion, tu_uint32 minorVersion, tu_uint32 patchVersion)
    : m_priv(std::make_shared<Priv>())
{
    m_priv->majorVersion = majorVersion;
    m_priv->minorVersion = minorVersion;
    m_priv->patchVersion = patchVersion;
}

zuri_packager::PackageVersion::PackageVersion(const PackageVersion &other)
    : m_priv(other.m_priv)
{
}

bool
zuri_packager::PackageVersion::isValid() const
{
    return m_priv != nullptr;
}

tu_uint32
zuri_packager::PackageVersion::getMajorVersion() const
{
    if (isValid())
        return m_priv->majorVersion;
    return 0;
}

tu_uint32
zuri_packager::PackageVersion::getMinorVersion() const
{
    if (isValid())
        return m_priv->minorVersion;
    return 0;
}

tu_uint32
zuri_packager::PackageVersion::getPatchVersion() const
{
    if (isValid())
        return m_priv->patchVersion;
    return 0;
}

std::string
zuri_packager::PackageVersion::toString() const
{
    if (isValid())
        return absl::StrCat(
            m_priv->majorVersion,
            ".",
            m_priv->minorVersion,
            ".",
            m_priv->patchVersion);
    return {};
}

inline int
compare_version_number(tu_uint32 lhs, tu_uint32 rhs)
{
    if (lhs < rhs)
        return -1;
    return lhs == rhs? 0 : 1;
}

inline int
compare_version(const zuri_packager::PackageVersion &lhs, const zuri_packager::PackageVersion &rhs)
{
    int cmp = compare_version_number(lhs.getMajorVersion(), rhs.getMajorVersion());
    if (cmp != 0)
        return cmp;
    cmp = compare_version_number(lhs.getMinorVersion(), rhs.getMinorVersion());
    if (cmp != 0)
        return cmp;
    return compare_version_number(lhs.getPatchVersion(), rhs.getPatchVersion());
}

int
zuri_packager::PackageVersion::compare(const PackageVersion &other) const
{
    if (m_priv) {
        if (!other.m_priv)
            return 1;
        return compare_version(*this, other);
    }
    return other.m_priv? -1 : 0;
}

bool
zuri_packager::PackageVersion::operator==(const PackageVersion &other) const
{
    return compare(other) == 0;
}

bool
zuri_packager::PackageVersion::operator!=(const PackageVersion &other) const
{
    return compare(other) != 0;
}

bool
zuri_packager::PackageVersion::operator<=(const PackageVersion &other) const
{
    return compare(other) <= 0;
}

bool
zuri_packager::PackageVersion::operator<(const PackageVersion &other) const
{
    return compare(other) < 0;
}

bool
zuri_packager::PackageVersion::operator>=(const PackageVersion &other) const
{
    return compare(other) >= 0;
}

bool
zuri_packager::PackageVersion::operator>(const PackageVersion &other) const
{
    return compare(other) > 0;
}

inline bool
parse_version_digit(std::string_view s, tu_uint32 &digit)
{
    if (s.empty())
        return false;
    auto first = s.front();
    if (s.size() == 1) {
        if (!std::isdigit(first))
            return false;
    } else {
        if (!std::isdigit(first) || first == '0')
            return false;
    }
    return absl::SimpleAtoi(s, &digit);
}

zuri_packager::PackageVersion
zuri_packager::PackageVersion::fromString(std::string_view s)
{
    // parse the version
    std::vector<std::string> versionParts = absl::StrSplit(s, '.');
    if (versionParts.size() != 3)
        return {};

    tu_uint32 major;
    tu_uint32 minor;
    tu_uint32 patch;

    if (!parse_version_digit(versionParts.at(0), major))
        return {};
    if (!parse_version_digit(versionParts.at(1), minor))
        return {};
    if (!parse_version_digit(versionParts.at(2), patch))
        return {};
    return PackageVersion(major, minor, patch);
}

zuri_packager::RequirementsMap::RequirementsMap()
    : m_requirements(std::make_shared<absl::flat_hash_map<PackageId,PackageVersion>>())
{
}

zuri_packager::RequirementsMap::RequirementsMap(const absl::flat_hash_map<PackageId,PackageVersion> &requirements)
    : m_requirements(std::make_shared<absl::flat_hash_map<PackageId,PackageVersion>>(requirements))
{
}

zuri_packager::RequirementsMap::RequirementsMap(const RequirementsMap &other)
    : m_requirements(other.m_requirements)
{
}

absl::flat_hash_map<zuri_packager::PackageId,zuri_packager::PackageVersion>::const_iterator
zuri_packager::RequirementsMap::requirementsBegin() const
{
    return m_requirements->cbegin();
}

absl::flat_hash_map<zuri_packager::PackageId,zuri_packager::PackageVersion>::const_iterator
zuri_packager::RequirementsMap::requirementsEnd() const
{
    return m_requirements->cend();
}

int zuri_packager::RequirementsMap::numRequirements() const
{
    return m_requirements->size();
}