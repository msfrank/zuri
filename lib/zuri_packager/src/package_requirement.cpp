
#include <zuri_packager/package_requirement.h>

zuri_packager::VersionRequirement::VersionRequirement(
    const PackageSpecifier &specifier,
    VersionComparison comparison)
    : m_specifier(specifier),
      m_comparison(comparison)
{
    TU_ASSERT (m_specifier.isValid());
    TU_ASSERT (m_comparison != VersionComparison::Invalid);
}

inline int
compare_version_number(tu_uint32 lhs, tu_uint32 rhs)
{
    if (lhs < rhs)
        return -1;
    return lhs == rhs? 0 : 1;
}

inline int
compare_version(const zuri_packager::PackageSpecifier &lhs, const zuri_packager::PackageSpecifier &rhs)
{
    int cmp = compare_version_number(lhs.getMajorVersion(), rhs.getMajorVersion());
    if (cmp != 0)
        return cmp;
    cmp = compare_version_number(lhs.getMinorVersion(), rhs.getMinorVersion());
    if (cmp != 0)
        return cmp;
    return compare_version_number(lhs.getPatchVersion(), rhs.getPatchVersion());
}

bool
zuri_packager::VersionRequirement::satisfiedBy(const PackageSpecifier &specifier) const
{
    if (specifier.getPackageDomain() != m_specifier.getPackageDomain())
        return false;
    if (specifier.getPackageName() != m_specifier.getPackageName())
        return false;

    auto cmp = compare_version(specifier, m_specifier);
    switch (m_comparison) {
        case VersionComparison::Equal:
            return cmp == 0;
        case VersionComparison::NotEqual:
            return cmp != 0;
        case VersionComparison::GreaterThan:
            return cmp > 0;
        case VersionComparison::GreaterOrEqual:
            return cmp >= 0;
        case VersionComparison::LesserThan:
            return cmp < 0;
        case VersionComparison::LesserOrEqual:
            return cmp <= 0;
        default:
            return false;
    }
}

std::shared_ptr<zuri_packager::AbstractPackageRequirement>
zuri_packager::VersionRequirement::create(
    const PackageSpecifier &specifier,
    VersionComparison comparison)
{
    auto req = std::shared_ptr<VersionRequirement>(new VersionRequirement(specifier, comparison));
    return std::static_pointer_cast<AbstractPackageRequirement>(req);
}
