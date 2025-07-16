
#include <zuri_distributor/distributor_result.h>
#include <zuri_distributor/static_package_resolver.h>

zuri_distributor::StaticPackageResolver::StaticPackageResolver(
    const absl::btree_map<zuri_packager::PackageSpecifier,PackageVersionDescriptor> &versions)
    : m_versions(versions)
{
}

tempo_utils::Result<zuri_distributor::PackageVersionDescriptor>
zuri_distributor::StaticPackageResolver::describePackageVersion(
    const zuri_packager::PackageId &packageId,
    const zuri_packager::PackageVersion &packageVersion)
{
    zuri_packager::PackageSpecifier specifier(packageId, packageVersion);
    auto entry = m_versions.find(specifier);
    if (entry == m_versions.cend())
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "missing package {}", specifier.toString());
    return entry->second;
}

tempo_utils::Result<std::shared_ptr<zuri_distributor::StaticPackageResolver>>
zuri_distributor::StaticPackageResolver::create(
    const absl::btree_map<zuri_packager::PackageSpecifier,PackageVersionDescriptor> &versions)
{
    auto resolver = std::shared_ptr<StaticPackageResolver>(new StaticPackageResolver(versions));
    return resolver;
}
