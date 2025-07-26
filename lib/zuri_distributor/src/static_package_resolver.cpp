
#include <tempo_utils/date_time.h>
#include <zuri_distributor/distributor_result.h>
#include <zuri_distributor/static_package_resolver.h>

zuri_distributor::StaticPackageResolver::StaticPackageResolver(
    const absl::btree_map<zuri_packager::PackageSpecifier,PackageDescriptor> &packages)
    : m_packages(packages)
{
    auto uploadDate = tempo_utils::seconds_since_epoch();

    absl::btree_map<zuri_packager::PackageId,CollectionDescriptor> collections;
    for (const auto &entry : packages) {
        auto packageId = entry.first.getPackageId();
        auto packageVersion = entry.first.getPackageVersion();

        CollectionDescriptor &collection = collections[packageId];
        CollectionDescriptor::Version version;
        version.uploadDateEpochMillis = uploadDate;
        version.pruned = false;
        collection.versions[packageVersion] = version;
    }

    RepositoryDescriptor repository;
    for (const auto &entry : collections) {
        auto &packageId = entry.first;
        RepositoryDescriptor::Collection collection;
        collection.description = packageId.toString();
        repository.collections[packageId] = std::move(collection);
    }
}

tempo_utils::Result<zuri_distributor::RepositoryDescriptor>
zuri_distributor::StaticPackageResolver::getRepository(std::string_view packageDomain)
{
    return m_repository;
}

tempo_utils::Result<zuri_distributor::CollectionDescriptor>
zuri_distributor::StaticPackageResolver::getCollection(const zuri_packager::PackageId &packageId)
{
    auto entry = m_collections.find(packageId);
    if (entry == m_collections.cend())
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "missing collection {}", packageId.toString());
    return entry->second;
}

tempo_utils::Result<zuri_distributor::PackageDescriptor>
zuri_distributor::StaticPackageResolver::getPackage(
    const zuri_packager::PackageId &packageId,
    const zuri_packager::PackageVersion &packageVersion)
{
    zuri_packager::PackageSpecifier specifier(packageId, packageVersion);
    auto entry = m_packages.find(specifier);
    if (entry == m_packages.cend())
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "missing package {}", specifier.toString());
    return entry->second;
}

tempo_utils::Result<std::shared_ptr<zuri_distributor::StaticPackageResolver>>
zuri_distributor::StaticPackageResolver::create(
    const absl::btree_map<zuri_packager::PackageSpecifier,PackageDescriptor> &packages)
{
    auto resolver = std::shared_ptr<StaticPackageResolver>(new StaticPackageResolver(packages));
    return resolver;
}
