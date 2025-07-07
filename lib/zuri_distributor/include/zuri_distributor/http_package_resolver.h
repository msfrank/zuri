#ifndef ZURI_DISTRIBUTOR_HTTP_PACKAGE_RESOLVER_H
#define ZURI_DISTRIBUTOR_HTTP_PACKAGE_RESOLVER_H

#include "abstract_package_resolver.h"
#include "distributor_result.h"

namespace zuri_distributor {

    class HttpPackageResolver : public AbstractPackageResolver {
    public:
        HttpPackageResolver();

        tempo_utils::Result<DependencySet> resolveDependencySet(
            absl::flat_hash_map<zuri_packager::PackageId,zuri_packager::RequirementsList> &dependencies) override;

    private:
        struct Priv;
        std::unique_ptr<Priv> m_priv;
    };
}

#endif // ZURI_DISTRIBUTOR_HTTP_PACKAGE_RESOLVER_H
