
#include <absl/strings/str_cat.h>
#include <absl/strings/str_join.h>
#include <absl/strings/str_split.h>

#include <zuri_packager/package_specifier.h>
#include <zuri_packager/package_types.h>

zuri_packager::PackageSpecifier::PackageSpecifier()
    : m_priv(std::make_shared<Priv>())
{
}

zuri_packager::PackageSpecifier::PackageSpecifier(
    const std::string &packageName,
    const std::string &packageDomain,
    tu_uint32 majorVersion,
    tu_uint32 minorVersion,
    tu_uint32 patchVersion)
{
    m_priv = std::make_shared<Priv>(packageName, packageDomain, majorVersion, minorVersion, patchVersion);
    TU_ASSERT (!m_priv->packageName.empty());
    TU_ASSERT (!m_priv->packageDomain.empty());
    TU_ASSERT (!(m_priv->majorVersion == 0 && m_priv->minorVersion == 0 && m_priv->patchVersion == 0));
}

zuri_packager::PackageSpecifier::PackageSpecifier(const PackageSpecifier &other)
    : m_priv(other.m_priv)
{
}

bool
zuri_packager::PackageSpecifier::isValid() const
{
    return !m_priv->packageName.empty() && !m_priv->packageDomain.empty();
}

std::string
zuri_packager::PackageSpecifier::getPackageName() const
{
    return m_priv->packageName;
}

std::string
zuri_packager::PackageSpecifier::getPackageDomain() const
{
    return m_priv->packageDomain;
}

tu_uint32
zuri_packager::PackageSpecifier::getMajorVersion() const
{
    return m_priv->majorVersion;
}

tu_uint32
zuri_packager::PackageSpecifier::getMinorVersion() const
{
    return m_priv->minorVersion;
}

tu_uint32
zuri_packager::PackageSpecifier::getPatchVersion() const
{
    return m_priv->patchVersion;
}

std::string
zuri_packager::PackageSpecifier::getVersionString() const
{
    return absl::StrCat(
        m_priv->majorVersion,
        ".",
        m_priv->minorVersion,
        ".",
        m_priv->patchVersion);
}

int
zuri_packager::PackageSpecifier::compare(const PackageSpecifier &other) const
{
    if (!m_priv && other.m_priv)
        return -1;
    if (m_priv && !other.m_priv)
        return 1;
    int domaincmp = m_priv->packageDomain.compare(other.m_priv->packageDomain);
    if (domaincmp != 0)
        return domaincmp;
    int namecmp = m_priv->packageName.compare(other.m_priv->packageName);
    if (namecmp != 0)
        return namecmp;
    if (m_priv->majorVersion != other.m_priv->majorVersion)
        return m_priv->majorVersion < other.m_priv->majorVersion? -1 : 1;
    if (m_priv->minorVersion != other.m_priv->minorVersion)
        return m_priv->minorVersion < other.m_priv->minorVersion? -1 : 1;
    if (m_priv->patchVersion != other.m_priv->patchVersion)
        return m_priv->patchVersion < other.m_priv->patchVersion? -1 : 1;
    return 0;
}

bool
zuri_packager::PackageSpecifier::operator==(const PackageSpecifier &other) const
{
    return compare(other) == 0;
}

bool
zuri_packager::PackageSpecifier::operator!=(const PackageSpecifier &other) const
{
    return compare(other) != 0;
}

bool
zuri_packager::PackageSpecifier::operator<(const PackageSpecifier &other) const
{
    return compare(other) < 0;
}

std::string
zuri_packager::PackageSpecifier::toString() const
{
    if (!isValid())
        return {};
    std::vector<std::string> hostParts = absl::StrSplit(m_priv->packageDomain, ".");
    std::reverse(hostParts.begin(), hostParts.end());
    return absl::StrCat(
        absl::StrJoin(hostParts, "."),
        "_",
        m_priv->packageName,
        "-",
        m_priv->majorVersion,
        ".",
        m_priv->minorVersion,
        ".",
        m_priv->patchVersion);
}

std::filesystem::path
zuri_packager::PackageSpecifier::toFilesystemPath(const std::filesystem::path &base) const
{
    if (!isValid())
        return {};
    auto path = base / toString();
    path += kPackageFileDotSuffix;
    return path;
}

tempo_utils::Url
zuri_packager::PackageSpecifier::toUrl() const
{
    if (!isValid())
        return {};
    auto username = absl::StrCat(
        m_priv->packageName,
        "-",
        m_priv->majorVersion,
        ".",
        m_priv->minorVersion,
        ".",
        m_priv->patchVersion);
    return tempo_utils::Url::fromOrigin(
        absl::StrCat("dev.zuri.pkg://", username, "@", m_priv->packageDomain));
}

zuri_packager::PackageSpecifier
zuri_packager::PackageSpecifier::fromString(const std::string &s)
{
    auto authority = tempo_utils::UrlAuthority::fromString(s);
    return fromAuthority(authority);
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

zuri_packager::PackageSpecifier
zuri_packager::PackageSpecifier::fromAuthority(const tempo_utils::UrlAuthority &authority)
{
    if (!authority.isValid())
        return {};
    if (!authority.hasHost())
        return {};
    if (!authority.hasUsername())
        return {};

    auto packageDomain = authority.getHost();

    // parse the name and version from userinfo
    auto nameAndVersion = authority.getUsername();
    auto lastDash = nameAndVersion.rfind('-');
    if (lastDash == std::string::npos)
        return {};
    auto packageName = nameAndVersion.substr(0, lastDash);
    auto versionString = nameAndVersion.substr(lastDash + 1);

    // parse the version
    std::vector<std::string> versionParts = absl::StrSplit(versionString, '.');
    if (versionParts.size() != 3)
        return {};

    tu_uint32 majorVersion;
    tu_uint32 minorVersion;
    tu_uint32 patchVersion;

    if (!parse_version_digit(versionParts.at(0), majorVersion))
        return {};
    if (!parse_version_digit(versionParts.at(1), minorVersion))
        return {};
    if (!parse_version_digit(versionParts.at(2), patchVersion))
        return {};

    return PackageSpecifier(packageName, packageDomain, majorVersion, minorVersion, patchVersion);
}

zuri_packager::PackageSpecifier
zuri_packager::PackageSpecifier::fromUrl(const tempo_utils::Url &uri)
{
    if (!uri.isValid())
        return {};
    if (uri.schemeView() != "dev.zuri.pkg")
        return {};
    return fromAuthority(uri.toAuthority());
}

zuri_packager::PackageSpecifier
zuri_packager::PackageSpecifier::fromFilesystemName(const std::filesystem::path &name)
{
    if (name.empty())
        return {};
    auto nameString = name.string();

    // construct the domain view
    auto underscore_index = nameString.find('_');
    if (underscore_index == std::string_view::npos)
        return {};
    auto domain_len = underscore_index;
    std::string_view domainView(nameString.data(), domain_len);

    // construct the name view
    const char *name_ptr = nameString.data() + underscore_index + 1;
    auto dash_index = nameString.find('-', underscore_index);
    if (dash_index == std::string_view::npos)
        return {};
    auto name_len = dash_index - underscore_index - 1;
    std::string_view nameView(name_ptr, name_len);

    // construct the version view
    const char *version_ptr = name_ptr + name_len + 1;
    std::string_view versionView(version_ptr);

    // extract the version numbers
    std::vector<std::string> versionNumbers = absl::StrSplit(versionView, ".");
    if (versionNumbers.size() != 3)
        return {};
    tu_uint32 majorVersion, minorVersion, patchVersion;
    if (!absl::SimpleAtoi(versionNumbers[0], &majorVersion))
        return {};
    if (!absl::SimpleAtoi(versionNumbers[1], &minorVersion))
        return {};
    if (!absl::SimpleAtoi(versionNumbers[2], &patchVersion))
        return {};

    // reverse the domain
    std::vector<std::string> domainLabels = absl::StrSplit(domainView, ".");
    std::reverse(domainLabels.begin(), domainLabels.end());
    auto packageDomain = absl::StrJoin(domainLabels, ".");

    return PackageSpecifier(
        std::string(nameView),
        packageDomain,
        majorVersion,
        minorVersion,
        patchVersion);
}
