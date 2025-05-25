
#include <zuri_packager/package_dependency.h>

zuri_packager::PackageDependency::PackageDependency()
    : m_priv(std::make_shared<Priv>())
{
}

zuri_packager::PackageDependency::PackageDependency(
    const std::string &name,
    const std::string &domain,
    const std::vector<std::shared_ptr<AbstractPackageRequirement>> &requirements)
{
    m_priv = std::make_shared<Priv>(name, domain, requirements);
    TU_ASSERT (!m_priv->name.empty());
    TU_ASSERT (!m_priv->domain.empty());
    TU_ASSERT (!m_priv->requirements.empty());
}

zuri_packager::PackageDependency::PackageDependency(const PackageDependency &other)
    : m_priv(other.m_priv)
{
}

bool
zuri_packager::PackageDependency::isValid() const
{
    return !m_priv->name.empty() && !m_priv->domain.empty();
}

std::string
zuri_packager::PackageDependency::getName() const
{
    return m_priv->name;
}

std::string
zuri_packager::PackageDependency::getDomain() const
{
    return m_priv->domain;
}

std::vector<std::shared_ptr<zuri_packager::AbstractPackageRequirement>>::const_iterator
zuri_packager::PackageDependency::requirementsBegin() const
{
    return m_priv->requirements.cbegin();
}

std::vector<std::shared_ptr<zuri_packager::AbstractPackageRequirement>>::const_iterator
zuri_packager::PackageDependency::requirementsEnd() const
{
    return m_priv->requirements.cend();
}

int
zuri_packager::PackageDependency::numRequirements() const
{
    return m_priv->requirements.size();
}

bool
zuri_packager::PackageDependency::satisfiedBy(const PackageSpecifier &specifier) const
{
    if (!isValid())
        return false;
    if (specifier.getPackageDomain() != m_priv->domain)
        return false;
    if (specifier.getPackageName() != m_priv->name)
        return false;

    for (const auto &requirement : m_priv->requirements) {
        if (!requirement->satisfiedBy(specifier))
            return false;
    }
    return true;
}
