#ifndef ZURI_DISTRIBUTOR_STATIC_PACKAGE_RESOLVER_H
#define ZURI_DISTRIBUTOR_STATIC_PACKAGE_RESOLVER_H

#include <absl/container/btree_map.h>

#include <zuri_packager/package_specifier.h>

#include "abstract_package_resolver.h"

namespace zuri_distributor {

    class StaticPackageResolver : public AbstractPackageResolver {
    public:
        static tempo_utils::Result<std::shared_ptr<StaticPackageResolver>> create(
            const absl::btree_map<zuri_packager::PackageSpecifier,PackageVersionDescriptor> &versions);

        tempo_utils::Result<PackageVersionDescriptor> describePackageVersion(
            const zuri_packager::PackageId &packageId,
            const zuri_packager::PackageVersion &packageVersion) override;

    private:
        absl::btree_map<zuri_packager::PackageSpecifier,PackageVersionDescriptor> m_versions;

        explicit StaticPackageResolver(
            const absl::btree_map<zuri_packager::PackageSpecifier,PackageVersionDescriptor> &versions);
    };
}

#endif // ZURI_DISTRIBUTOR_STATIC_PACKAGE_RESOLVER_H
