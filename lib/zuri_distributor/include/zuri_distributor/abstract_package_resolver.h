#ifndef ZURI_DISTRIBUTOR_ABSTRACT_PACKAGE_RESOLVER_H
#define ZURI_DISTRIBUTOR_ABSTRACT_PACKAGE_RESOLVER_H

#include <tempo_utils/result.h>

#include "dependency_set.h"

namespace zuri_distributor {

    struct RepositoryDescriptor {
        struct Collection {
            std::string description;
        };
        absl::flat_hash_map<zuri_packager::PackageId,Collection> collections;
    };

    struct CollectionDescriptor {
        zuri_packager::PackageId id;
        struct Version {
            tu_int64 uploadDateEpochMillis = 0;
            bool pruned = false;
        };
        absl::flat_hash_map<zuri_packager::PackageVersion,Version> versions;
    };

    struct PackageDescriptor {
        zuri_packager::PackageId id;
        zuri_packager::PackageVersion version;
        absl::flat_hash_set<zuri_packager::PackageSpecifier> dependencies;
        tempo_utils::Url url;
        tu_int64 uploadDateEpochMillis = 0;
        bool pruned = false;
    };

    class AbstractPackageResolver {
    public:
        virtual ~AbstractPackageResolver() = default;

        /**
         * Get the list of packages in the repository.
         *
         * @return
         */
        virtual tempo_utils::Result<RepositoryDescriptor> getRepository(std::string_view packageDomain) = 0;

        /**
         * Get the list of package versions for the specified package id.
         *
         * @param packageId
         * @return
         */
        virtual tempo_utils::Result<CollectionDescriptor> getCollection(
            const zuri_packager::PackageId &packageId) = 0;

        /**
         * Get the package.
         *
         * @param packageId
         * @param packageVersion
         * @return
         */
        virtual tempo_utils::Result<PackageDescriptor> getPackage(
            const zuri_packager::PackageId &packageId,
            const zuri_packager::PackageVersion &packageVersion) = 0;
    };
}

#endif // ZURI_DISTRIBUTOR_ABSTRACT_PACKAGE_RESOLVER_H
