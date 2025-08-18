#ifndef ZURI_DISTRIBUTOR_PACKAGE_FETCHER_H
#define ZURI_DISTRIBUTOR_PACKAGE_FETCHER_H

#include <filesystem>
#include <memory>

#include <tempo_utils/result.h>
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
        tempo_utils::Url url;
        std::string id;
        std::filesystem::path path;
        tempo_utils::Status status;
    };

    class PackageFetcher {
    public:
        explicit PackageFetcher(const PackageFetcherOptions &options = {});
        ~PackageFetcher();

        tempo_utils::Status configure();

        tempo_utils::Status requestFile(const tempo_utils::Url &url, std::string_view id);
        tempo_utils::Result<std::string> requestFile(const tempo_utils::Url &url);
        tempo_utils::Status fetchFiles();

        bool hasResult(std::string_view id) const;
        FetchResult getResult(std::string_view id) const;
        absl::flat_hash_map<std::string,FetchResult>::const_iterator resultsBegin() const;
        absl::flat_hash_map<std::string,FetchResult>::const_iterator resultsEnd() const;
        int numResults() const;

    private:
        PackageFetcherOptions m_options;

        struct Priv;
        std::unique_ptr<Priv> m_priv;
        absl::flat_hash_map<std::string,FetchResult> m_results;
    };
}

#endif // ZURI_DISTRIBUTOR_PACKAGE_FETCHER_H
