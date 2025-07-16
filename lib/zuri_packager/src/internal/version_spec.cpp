
#include <zuri_packager/internal/version_spec.h>

zuri_packager::internal::VersionSpec::VersionSpec()
    : m_priv(std::make_shared<Priv>())
{
    m_priv->type = VersionSpecType::AnyVersion;
}

zuri_packager::internal::VersionSpec::VersionSpec(tu_uint32 majorVersion)
    : m_priv(std::make_shared<Priv>())
{
    m_priv->type = VersionSpecType::MajorVersion;
    m_priv->majorVersion = Option(majorVersion);
}

zuri_packager::internal::VersionSpec::VersionSpec(tu_uint32 majorVersion, tu_uint32 minorVersion)
    : m_priv(std::make_shared<Priv>())
{
    m_priv->type = VersionSpecType::ApiVersion;
    m_priv->majorVersion = Option(majorVersion);
    m_priv->minorVersion = Option(minorVersion);
}

zuri_packager::internal::VersionSpec::VersionSpec(tu_uint32 majorVersion, tu_uint32 minorVersion, tu_uint32 patchVersion)
    : m_priv(std::make_shared<Priv>())
{
    m_priv->type = VersionSpecType::FullVersion;
    m_priv->majorVersion = Option(majorVersion);
    m_priv->minorVersion = Option(minorVersion);
    m_priv->patchVersion = Option(patchVersion);
}

zuri_packager::internal::VersionSpec::VersionSpec(const VersionSpec &other)
    : m_priv(other.m_priv)
{
}

zuri_packager::internal::VersionSpecType
zuri_packager::internal::VersionSpec::getType() const
{
    return m_priv->type;
}

bool
zuri_packager::internal::VersionSpec::hasMajorVersion() const
{
    return m_priv->majorVersion.hasValue();
}

tu_uint32
zuri_packager::internal::VersionSpec::getMajorVersion() const
{
    return m_priv->majorVersion.getOrDefault(0);
}

bool
zuri_packager::internal::VersionSpec::hasMinorVersion() const
{
    return m_priv->minorVersion.hasValue();
}

tu_uint32
zuri_packager::internal::VersionSpec::getMinorVersion() const
{
    return m_priv->minorVersion.getOrDefault(0);
}

bool
zuri_packager::internal::VersionSpec::hasPatchVersion() const
{
    return m_priv->patchVersion.hasValue();
}

tu_uint32
zuri_packager::internal::VersionSpec::getPatchVersion() const
{
    return m_priv->patchVersion.getOrDefault(0);
}

zuri_packager::PackageVersion
zuri_packager::internal::VersionSpec::toVersion() const
{
    return PackageVersion(
        m_priv->majorVersion.getOrDefault(0),
        m_priv->minorVersion.getOrDefault(0),
        m_priv->patchVersion.getOrDefault(0));
}
