
#include <zuri_distributor/http_package_resolver.h>

struct zuri_distributor::HttpPackageResolver::Priv {
};

zuri_distributor::HttpPackageResolver::HttpPackageResolver()
    : m_priv(std::make_unique<Priv>())
{
}

tempo_utils::Result<zuri_distributor::DependencySet>
zuri_distributor::HttpPackageResolver::resolveDependencySet(
    absl::flat_hash_map<zuri_packager::PackageId,zuri_packager::RequirementsList> &dependencies)
{
    return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
        "unimplemented");
}
