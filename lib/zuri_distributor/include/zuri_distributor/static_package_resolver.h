#ifndef ZURI_DISTRIBUTOR_STATIC_PACKAGE_RESOLVER_H
#define ZURI_DISTRIBUTOR_STATIC_PACKAGE_RESOLVER_H

#include <absl/container/btree_map.h>

#include <zuri_packager/package_specifier.h>

#include "abstract_package_resolver.h"

namespace zuri_distributor {

    class StaticPackageResolver : public AbstractPackageResolver {
    public:
        static tempo_utils::Result<std::shared_ptr<StaticPackageResolver>> create(
            const absl::btree_map<zuri_packager::PackageSpecifier,PackageDescriptor> &packages);

        tempo_utils::Result<RepositoryDescriptor> getRepository(std::string_view packageDomain) override;

        tempo_utils::Result<CollectionDescriptor> getCollection(
            const zuri_packager::PackageId &packageId) override;

        tempo_utils::Result<PackageDescriptor> getPackage(
            const zuri_packager::PackageId &packageId,
            const zuri_packager::PackageVersion &packageVersion) override;

    private:
        RepositoryDescriptor m_repository;
        absl::btree_map<zuri_packager::PackageId,CollectionDescriptor> m_collections;
        absl::btree_map<zuri_packager::PackageSpecifier,PackageDescriptor> m_packages;

        StaticPackageResolver(
            const absl::btree_map<zuri_packager::PackageSpecifier,PackageDescriptor> &packages);
    };
}

#endif // ZURI_DISTRIBUTOR_STATIC_PACKAGE_RESOLVER_H
