
#include <zuri_packager/package_dependency.h>

zuri_packager::PackageDependency::PackageDependency()
    : m_priv(std::make_shared<Priv>())
{
}

zuri_packager::PackageDependency::PackageDependency(
    const PackageId &packageId,
    const RequirementsList &requirements)
    : m_priv(std::make_shared<Priv>())
{
    m_priv->id = packageId;
    m_priv->requirements = requirements;
    TU_ASSERT (m_priv->id.isValid());
}

zuri_packager::PackageDependency::PackageDependency(
    const std::string &packageName,
    const std::string &packageDomain,
    const std::vector<std::shared_ptr<AbstractPackageRequirement>> &requirements)
    : m_priv(std::make_shared<Priv>())
{
    m_priv->id = PackageId(packageName, packageDomain);
    m_priv->requirements = RequirementsList(requirements);
    TU_ASSERT (m_priv->id.isValid());
}

zuri_packager::PackageDependency::PackageDependency(const PackageDependency &other)
    : m_priv(other.m_priv)
{
}

bool
zuri_packager::PackageDependency::isValid() const
{
    return m_priv->id.isValid();
}

zuri_packager::PackageId
zuri_packager::PackageDependency::getPackageId() const
{
    if (isValid())
        return m_priv->id;
    return {};
}

zuri_packager::RequirementsList
zuri_packager::PackageDependency::getRequirements() const
{
    if (isValid())
        return m_priv->requirements;
    return {};
}

std::string
zuri_packager::PackageDependency::getName() const
{
    if (isValid())
        return m_priv->id.getName();
    return {};
}

std::string
zuri_packager::PackageDependency::getDomain() const
{
    if (isValid())
        return m_priv->id.getDomain();
    return {};
}

std::vector<std::shared_ptr<zuri_packager::AbstractPackageRequirement>>::const_iterator
zuri_packager::PackageDependency::requirementsBegin() const
{
    return m_priv->requirements.requirementsBegin();
}

std::vector<std::shared_ptr<zuri_packager::AbstractPackageRequirement>>::const_iterator
zuri_packager::PackageDependency::requirementsEnd() const
{
    return m_priv->requirements.requirementsEnd();
}

int
zuri_packager::PackageDependency::numRequirements() const
{
    return m_priv->requirements.numRequirements();
}

bool
zuri_packager::PackageDependency::satisfiedBy(const PackageSpecifier &specifier) const
{
    if (!isValid())
        return false;
    if (m_priv->id != specifier.getPackageId())
        return false;
    auto version = specifier.getPackageVersion();
    for (auto it = m_priv->requirements.requirementsBegin(); it != m_priv->requirements.requirementsEnd(); ++it) {
        auto &requirement = *it;
        if (!requirement->satisfiedBy(version))
            return false;
    }
    return true;
}

tempo_config::ConfigNode
zuri_packager::PackageDependency::toNode() const
{
    auto numRequirements = m_priv->requirements.numRequirements();
    if (numRequirements == 0)
        return tempo_config::startSeq().buildNode();

    auto it = m_priv->requirements.requirementsBegin();

    if (m_priv->requirements.numRequirements() == 1) {
        auto node = (*it)->toNode();
        if (node.getNodeType() == tempo_config::ConfigNodeType::kValue)
            return node;
        return {};
    }

    auto requirementsBuilder = tempo_config::startSeq();
    for (; it != m_priv->requirements.requirementsEnd(); ++it) {
        auto node = (*it)->toNode();
        if (node.getNodeType() != tempo_config::ConfigNodeType::kValue)
            return {};
        requirementsBuilder = requirementsBuilder.append(node);
    }
    return requirementsBuilder.buildNode();
}