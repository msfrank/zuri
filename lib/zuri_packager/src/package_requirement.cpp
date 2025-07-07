
#include <tempo_config/config_result.h>
#include <zuri_packager/package_requirement.h>

zuri_packager::ExactVersionRequirement::ExactVersionRequirement(const PackageVersion &version)
    : m_version(version)
{
    TU_ASSERT (m_version.isValid());
}

zuri_packager::RequirementType
zuri_packager::ExactVersionRequirement::getType() const
{
    return RequirementType::ExactVersion;
}

zuri_packager::VersionInterval
zuri_packager::ExactVersionRequirement::getInterval() const
{
    PackageVersion openUpperBound(m_version.getMajorVersion(), m_version.getMinorVersion(),
        m_version.getPatchVersion() + 1);
    return VersionInterval(m_version, openUpperBound);
}

bool
zuri_packager::ExactVersionRequirement::satisfiedBy(const PackageVersion &version) const
{
    return m_version == version;
}

tempo_config::ConfigNode
zuri_packager::ExactVersionRequirement::toNode() const
{
    return tempo_config::valueNode(absl::StrCat("=", m_version.toString()));
}

std::shared_ptr<zuri_packager::AbstractPackageRequirement>
zuri_packager::ExactVersionRequirement::create(const PackageVersion &packageVersion)
{
    auto req = std::shared_ptr<ExactVersionRequirement>(new ExactVersionRequirement(packageVersion));
    return std::static_pointer_cast<AbstractPackageRequirement>(req);
}

zuri_packager::VersionRangeRequirement::VersionRangeRequirement(
    const PackageVersion &lowerBound,
    const PackageVersion &upperBound)
    : m_lowerBound(lowerBound),
      m_upperBound(upperBound)
{
    TU_ASSERT (m_lowerBound.isValid());
    TU_ASSERT (m_upperBound.isValid());
}

zuri_packager::RequirementType
zuri_packager::VersionRangeRequirement::getType() const
{
    return RequirementType::VersionRange;
}

zuri_packager::VersionInterval
zuri_packager::VersionRangeRequirement::getInterval() const
{
    PackageVersion openUpperBound(m_upperBound.getMajorVersion(), m_upperBound.getMinorVersion(),
        m_upperBound.getPatchVersion() + 1);
    return VersionInterval(m_lowerBound, openUpperBound);
}

bool
zuri_packager::VersionRangeRequirement::satisfiedBy(const PackageVersion &version) const
{
    return m_lowerBound <= version && version <= m_upperBound;
}

tempo_config::ConfigNode
zuri_packager::VersionRangeRequirement::toNode() const
{
    return tempo_config::valueNode(absl::StrCat(
    m_lowerBound.toString(),
    "-",
    m_upperBound.toString()));
}

std::shared_ptr<zuri_packager::AbstractPackageRequirement>
zuri_packager::VersionRangeRequirement::create(
     const PackageVersion &lowerBound,
     const PackageVersion &upperBound)
{
    auto req = std::shared_ptr<VersionRangeRequirement>(new VersionRangeRequirement(
        lowerBound, upperBound));
    return std::static_pointer_cast<AbstractPackageRequirement>(req);
}

zuri_packager::RequirementsList::RequirementsList()
    : m_priv(std::make_shared<Priv>())
{
}

zuri_packager::RequirementsList::RequirementsList(
    const std::vector<std::shared_ptr<AbstractPackageRequirement>> &requirements)
    : m_priv(std::make_shared<Priv>(requirements))
{
}

zuri_packager::RequirementsList::RequirementsList(const RequirementsList &other)
    : m_priv(other.m_priv)
{
}

std::vector<std::shared_ptr<zuri_packager::AbstractPackageRequirement>>::const_iterator
zuri_packager::RequirementsList::requirementsBegin() const
{
    return m_priv->requirements.cbegin();
}

std::vector<std::shared_ptr<zuri_packager::AbstractPackageRequirement>>::const_iterator
zuri_packager::RequirementsList::requirementsEnd() const
{
    return m_priv->requirements.cend();
}

int
zuri_packager::RequirementsList::numRequirements() const
{
    return m_priv->requirements.size();
}
