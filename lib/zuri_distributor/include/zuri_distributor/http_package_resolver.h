#ifndef ZURI_DISTRIBUTOR_HTTP_PACKAGE_RESOLVER_H
#define ZURI_DISTRIBUTOR_HTTP_PACKAGE_RESOLVER_H
#include "abstract_package_resolver.h"

namespace zuri_distributor {

    struct HttpPackageResolverOptions {
    };

    class HttpPackageResolver : public AbstractPackageResolver {
    public:
        static tempo_utils::Result<std::shared_ptr<HttpPackageResolver>> create(
            const HttpPackageResolverOptions &options = {});

        tempo_utils::Result<RepositoryDescriptor> getRepository(std::string_view packageDomain) override;

        tempo_utils::Result<CollectionDescriptor> getCollection(
            const zuri_packager::PackageId &packageId) override;

        tempo_utils::Result<PackageDescriptor> getPackage(
            const zuri_packager::PackageId &packageId,
            const zuri_packager::PackageVersion &packageVersion) override;

    private:
        struct Priv;
        std::unique_ptr<Priv> m_priv;

        explicit HttpPackageResolver(std::unique_ptr<Priv> priv);

        tempo_utils::Result<tempo_utils::Url> resolveBaseUri(std::string_view domain);
        tempo_utils::Result<tempo_utils::Url> resolveLocation(
            std::string_view domain,
            const tempo_utils::UrlPath &path);

        enum class ErrorMode {
            Default,
            IgnoreClientErrors,
            IgnoreClientAndServerErrors,
        };
        tempo_utils::Status performGet(
            const tempo_utils::Url &url,
            ErrorMode errorMode = ErrorMode::Default);
    };
}

#endif // ZURI_DISTRIBUTOR_HTTP_PACKAGE_RESOLVER_H
