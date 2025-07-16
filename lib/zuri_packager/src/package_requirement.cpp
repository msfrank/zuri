
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
    return tempo_config::valueNode(m_version.toString());
}

std::shared_ptr<zuri_packager::AbstractPackageRequirement>
zuri_packager::ExactVersionRequirement::create(const PackageVersion &packageVersion)
{
    auto req = std::shared_ptr<ExactVersionRequirement>(new ExactVersionRequirement(packageVersion));
    return std::static_pointer_cast<AbstractPackageRequirement>(req);
}

std::shared_ptr<zuri_packager::AbstractPackageRequirement>
zuri_packager::ExactVersionRequirement::create(tu_uint32 major, tu_uint32 minor, tu_uint32 patch)
{
    return create(PackageVersion(major, minor, patch));
}

zuri_packager::HyphenRangeRequirement::HyphenRangeRequirement(
    const PackageVersion &lowerBound,
    const PackageVersion &upperBound)
    : m_lowerBound(lowerBound),
      m_upperBound(upperBound)
{
    TU_ASSERT (m_lowerBound.isValid());
    TU_ASSERT (m_upperBound.isValid());
}

zuri_packager::RequirementType
zuri_packager::HyphenRangeRequirement::getType() const
{
    return RequirementType::HyphenRange;
}

zuri_packager::VersionInterval
zuri_packager::HyphenRangeRequirement::getInterval() const
{
    PackageVersion openUpperBound(m_upperBound.getMajorVersion(), m_upperBound.getMinorVersion(),
        m_upperBound.getPatchVersion() + 1);
    return VersionInterval(m_lowerBound, openUpperBound);
}

bool
zuri_packager::HyphenRangeRequirement::satisfiedBy(const PackageVersion &version) const
{
    return m_lowerBound <= version && version <= m_upperBound;
}

tempo_config::ConfigNode
zuri_packager::HyphenRangeRequirement::toNode() const
{
    return tempo_config::valueNode(absl::StrCat(
    m_lowerBound.toString(),
    "-",
    m_upperBound.toString()));
}

std::shared_ptr<zuri_packager::AbstractPackageRequirement>
zuri_packager::HyphenRangeRequirement::create(
     const PackageVersion &lowerBound,
     const PackageVersion &upperBound)
{
    auto req = std::shared_ptr<HyphenRangeRequirement>(new HyphenRangeRequirement(
        lowerBound, upperBound));
    return std::static_pointer_cast<AbstractPackageRequirement>(req);
}

zuri_packager::TildeRangeRequirement::TildeRangeRequirement(
    const VersionInterval &interval,
    const tempo_config::ConfigNode &node)
    : m_interval(interval),
      m_node(node)
{
}

zuri_packager::RequirementType
zuri_packager::TildeRangeRequirement::getType() const
{
    return RequirementType::TildeRange;
}

zuri_packager::VersionInterval
zuri_packager::TildeRangeRequirement::getInterval() const
{
    return m_interval;
}

bool
zuri_packager::TildeRangeRequirement::satisfiedBy(const PackageVersion &version) const
{
    auto interval = getInterval();
    return interval.closedLowerBound <= version && version < interval.openUpperBound;
}

tempo_config::ConfigNode
zuri_packager::TildeRangeRequirement::toNode() const
{
    return m_node;
}

std::shared_ptr<zuri_packager::AbstractPackageRequirement>
zuri_packager::TildeRangeRequirement::create(const PackageVersion &packageVersion)
{
    return create(packageVersion.getMajorVersion(), packageVersion.getMinorVersion(), packageVersion.getPatchVersion());
}

std::shared_ptr<zuri_packager::AbstractPackageRequirement>
zuri_packager::TildeRangeRequirement::create(tu_uint32 major, tu_uint32 minor, tu_uint32 patch)
{
    PackageVersion closedLowerBound(major, minor, patch);

    // e.g. 1.2.3 or 1.2.0 or 1.0.0 or 0.1.2 or 0.0.1 or 0.0.0
    PackageVersion openUpperBound = PackageVersion(major, minor + 1, 0);

    auto req = std::shared_ptr<TildeRangeRequirement>(new TildeRangeRequirement(
        VersionInterval(closedLowerBound, openUpperBound),
        tempo_config::valueNode(absl::StrCat("~", major, ".", minor, ".", patch))));
    return std::static_pointer_cast<AbstractPackageRequirement>(req);
}

std::shared_ptr<zuri_packager::AbstractPackageRequirement>
zuri_packager::TildeRangeRequirement::create(tu_uint32 major, tu_uint32 minor)
{
    PackageVersion closedLowerBound(major, minor, 0);

    // e.g. 1.2, 0.1, 0.0
    PackageVersion openUpperBound = PackageVersion(major, minor + 1, 0);

    auto req = std::shared_ptr<TildeRangeRequirement>(new TildeRangeRequirement(
        VersionInterval(closedLowerBound, openUpperBound),
        tempo_config::valueNode(absl::StrCat("~", major, ".", minor))));
    return std::static_pointer_cast<AbstractPackageRequirement>(req);
}

std::shared_ptr<zuri_packager::AbstractPackageRequirement>
zuri_packager::TildeRangeRequirement::create(tu_uint32 major)
{
    PackageVersion closedLowerBound(major, 0, 0);

    // e.g. 1, 0
    PackageVersion openUpperBound = PackageVersion(major + 1, 0, 0);

    auto req = std::shared_ptr<TildeRangeRequirement>(new TildeRangeRequirement(
        VersionInterval(closedLowerBound, openUpperBound),
        tempo_config::valueNode(absl::StrCat("~", major))));
    return std::static_pointer_cast<AbstractPackageRequirement>(req);
}

zuri_packager::CaretRangeRequirement::CaretRangeRequirement(
    const VersionInterval &interval,
    const tempo_config::ConfigNode &node)
    : m_interval(interval),
      m_node(node)
{
}

zuri_packager::RequirementType
zuri_packager::CaretRangeRequirement::getType() const
{
    return RequirementType::CaretRange;
}

zuri_packager::VersionInterval
zuri_packager::CaretRangeRequirement::getInterval() const
{
    return m_interval;
}

bool
zuri_packager::CaretRangeRequirement::satisfiedBy(const PackageVersion &version) const
{
    return m_interval.closedLowerBound <= version && version < m_interval.openUpperBound;
}

tempo_config::ConfigNode
zuri_packager::CaretRangeRequirement::toNode() const
{
    return m_node;
}

std::shared_ptr<zuri_packager::AbstractPackageRequirement>
zuri_packager::CaretRangeRequirement::create(const PackageVersion &packageVersion)
{
    return create(packageVersion.getMajorVersion(), packageVersion.getMinorVersion(), packageVersion.getPatchVersion());
}

std::shared_ptr<zuri_packager::AbstractPackageRequirement>
zuri_packager::CaretRangeRequirement::create(tu_uint32 major, tu_uint32 minor, tu_uint32 patch)
{
    PackageVersion closedLowerBound(major, minor, patch);

    PackageVersion openUpperBound;
    if (major > 0) {
        // e.g. 1.2.3 or 1.2.0 or 1.0.0
        openUpperBound = PackageVersion(major + 1, 0, 0);
    } else if (minor > 0) {
        // e.g. 0.1.2 or 0.1.0
        openUpperBound = PackageVersion(0, minor + 1, 0);
    } else {
        // e.g. 0.0.1
        openUpperBound = PackageVersion(0, 0, patch + 1);
    }

    auto req = std::shared_ptr<CaretRangeRequirement>(new CaretRangeRequirement(
        VersionInterval(closedLowerBound, openUpperBound),
        tempo_config::valueNode(absl::StrCat("^", major, ".", minor, ".", patch))));
    return std::static_pointer_cast<AbstractPackageRequirement>(req);
}

std::shared_ptr<zuri_packager::AbstractPackageRequirement>
zuri_packager::CaretRangeRequirement::create(tu_uint32 major, tu_uint32 minor)
{
    PackageVersion closedLowerBound(major, minor, 0);

    PackageVersion openUpperBound;
    if (major > 0) {
        // e.g. 1.2 or 1.0
        openUpperBound = PackageVersion(major + 1, 0, 0);
    } else {
        // e.g. 0.1 or 0.0
        openUpperBound = PackageVersion(0, minor + 1, 0);
    }

    auto req = std::shared_ptr<CaretRangeRequirement>(new CaretRangeRequirement(
        VersionInterval(closedLowerBound, openUpperBound),
        tempo_config::valueNode(absl::StrCat("^", major, ".", minor))));
    return std::static_pointer_cast<AbstractPackageRequirement>(req);
}

std::shared_ptr<zuri_packager::AbstractPackageRequirement>
zuri_packager::CaretRangeRequirement::create(tu_uint32 major)
{
    PackageVersion closedLowerBound(major, 0, 0);
    PackageVersion openUpperBound(major + 1, 0, 0);

    auto req = std::shared_ptr<CaretRangeRequirement>(new CaretRangeRequirement(
        VersionInterval(closedLowerBound, openUpperBound),
        tempo_config::valueNode(absl::StrCat("^", major))));
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
