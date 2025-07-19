#ifndef ZURI_DISTRIBUTOR_PACKAGE_FETCHER_H
#define ZURI_DISTRIBUTOR_PACKAGE_FETCHER_H

#include <filesystem>
#include <memory>

#include <tempo_utils/status.h>
#include <zuri_packager/package_specifier.h>

namespace zuri_distributor {

    struct PackageFetcherOptions {
        /**
         *
         */
        std::filesystem::path downloadRoot = {};
        /**
         *
         */
        int pollTimeoutInMs = 100;
    };

    struct FetchResult {
        std::filesystem::path path;
        tempo_utils::Status status;
    };

    class PackageFetcher {
    public:
        explicit PackageFetcher(const PackageFetcherOptions &options = {});
        ~PackageFetcher();

        tempo_utils::Status configure();
        tempo_utils::Status addPackage(const zuri_packager::PackageSpecifier &specifier, const tempo_utils::Url &url);
        tempo_utils::Status fetchPackages();

        bool hasResult(const zuri_packager::PackageSpecifier &specifier) const;
        FetchResult getResult(const zuri_packager::PackageSpecifier &specifier) const;
        absl::flat_hash_map<zuri_packager::PackageSpecifier,FetchResult>::const_iterator resultsBegin() const;
        absl::flat_hash_map<zuri_packager::PackageSpecifier,FetchResult>::const_iterator resultsEnd() const;
        int numResults() const;

    private:
        PackageFetcherOptions m_options;

        struct Priv;
        std::unique_ptr<Priv> m_priv;
        absl::flat_hash_map<zuri_packager::PackageSpecifier,FetchResult> m_results;
    };
}

#endif // ZURI_DISTRIBUTOR_PACKAGE_FETCHER_H
