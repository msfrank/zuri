
#include <curl/curl.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/config_utils.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_config/time_conversions.h>

#include <zuri_distributor/distributor_result.h>
#include <zuri_distributor/http_package_resolver.h>

#include "zuri_packager/packaging_conversions.h"

struct Location {
    tempo_utils::Url baseUri;
    absl::Time expires;
};

struct Context {
    // config
    tempo_utils::Url defaultLocationUrl;

    // state
    CURL *handle = nullptr;
    absl::flat_hash_map<std::string,Location> locations;

    // per-request state
    long responseCode;
    std::string contentType;
    absl::Time lastModified;
    absl::Time expires;
    std::string body;

    void reset() {
        responseCode = 0;
        contentType.clear();
        lastModified = {};
        expires = {};
        body.clear();
    }

    ~Context() {
        if (handle != nullptr) {
            curl_easy_cleanup(handle);
        }
    }
};

struct zuri_distributor::HttpPackageResolver::Priv {
    Context ctx;
};

static size_t
write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    auto *ctx = (Context *) userdata;
    ctx->body.insert(ctx->body.end(), ptr, ptr + nmemb);
    return nmemb;
}

tempo_utils::Status
zuri_distributor::HttpPackageResolver::performGet(
    const tempo_utils::Url &url,
    ErrorMode errorMode)
{
    auto &ctx = m_priv->ctx;
    ctx.reset();

    curl_easy_setopt(ctx.handle, CURLOPT_URL, url.uriView());

    auto ret = curl_easy_perform(ctx.handle);
    if (ret != CURLE_OK)
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "curl_easy_perform failed: {}", curl_easy_strerror(ret));

    curl_easy_getinfo(ctx.handle, CURLINFO_RESPONSE_CODE, &ctx.responseCode);
    switch (errorMode) {
        case ErrorMode::Default:
            if (ctx.responseCode >= 400)
                return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
                    "unexpected {} response code", ctx.responseCode);
            break;
        case ErrorMode::IgnoreClientErrors:
            if (ctx.responseCode >= 500)
                return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
                    "unexpected {} response code", ctx.responseCode);
            break;
        case ErrorMode::IgnoreClientAndServerErrors:
            break;
    }

    curl_header *header;

    if(CURLHE_OK == curl_easy_header(
        ctx.handle, "Content-Type", 0, CURLH_HEADER, -1, &header)) {
        ctx.contentType = std::string(header->value);
    }
    if(CURLHE_OK == curl_easy_header(
        ctx.handle, "Last-Modified", 0, CURLH_HEADER, -1, &header)) {
        absl::ParseTime(absl::RFC1123_full, header->value, &ctx.lastModified, nullptr);
    }
    if(CURLHE_OK == curl_easy_header(
        ctx.handle, "Expires", 0, CURLH_HEADER, -1, &header)) {
        absl::ParseTime(absl::RFC1123_full, header->value, &ctx.expires, nullptr);
    }

    return {};
}

tempo_utils::Result<tempo_utils::Url>
zuri_distributor::HttpPackageResolver::resolveBaseUri(std::string_view domain)
{
    const auto &ctx = m_priv->ctx;
    auto now = absl::Now();

    // check locations cache
    auto entry = ctx.locations.find(domain);
    if (entry != ctx.locations.cend()) {
        const auto &location = entry->second;
        if (now < location.expires)
            return location.baseUri;
    }

    tempo_config::ConfigNode rootNode;

    // check well-known location for base URI
    auto wellKnownUrl = tempo_utils::Url::fromAbsolute(
        "https", domain, "/.well-known/zuri-pkgs/location.json");
    TU_RETURN_IF_NOT_OK (performGet(wellKnownUrl, ErrorMode::IgnoreClientAndServerErrors));
    if (ctx.responseCode != 200) {
        TU_RETURN_IF_NOT_OK (performGet(ctx.defaultLocationUrl, ErrorMode::IgnoreClientAndServerErrors));
        if (ctx.responseCode != 200)
            return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
                "repository location not found for {}", domain);
    }

    TU_ASSIGN_OR_RETURN (rootNode, tempo_config::read_config_string(ctx.body));
    auto rootMap = rootNode.toMap();

    Location location;

    tempo_config::UrlParser baseUriParser;
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(location.baseUri, baseUriParser, rootMap, "baseUri"));

    tempo_config::TimeParser expiresParser(absl::RFC3339_full, absl::Now() + absl::Hours(1));
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(location.expires, expiresParser, rootMap, "expires"));

    if (location.baseUri.isRelative()) {
        auto basePath = location.baseUri.toPath();
        location.baseUri = wellKnownUrl.resolve(location.baseUri);
    }

    m_priv->ctx.locations[domain] = location;
    return location.baseUri;
}

tempo_utils::Result<tempo_utils::Url>
zuri_distributor::HttpPackageResolver::resolveLocation(
    std::string_view domain,
    const tempo_utils::UrlPath &path)
{
    tempo_utils::Url baseUri;
    TU_ASSIGN_OR_RETURN (baseUri, resolveBaseUri(domain));
    return baseUri.traverse(path);
}

zuri_distributor::HttpPackageResolver::HttpPackageResolver(std::unique_ptr<Priv> priv)
    : m_priv(std::move(priv))
{
    TU_ASSERT (m_priv != nullptr);
}

class RepositoryCollectionParser
    : public tempo_config::AbstractConverter<zuri_distributor::RepositoryDescriptor::Collection> {
public:
    tempo_utils::Status convertValue(
        const tempo_config::ConfigNode &node,
        zuri_distributor::RepositoryDescriptor::Collection &value) const override
    {
        zuri_distributor::RepositoryDescriptor::Collection collection;

        auto map = node.toMap();
        tempo_config::StringParser descriptionParser;
        TU_RETURN_IF_NOT_OK (tempo_config::parse_config(collection.description, descriptionParser,
            map, "description"));

        value = std::move(collection);
        return {};
    }
};

tempo_utils::Result<zuri_distributor::RepositoryDescriptor>
zuri_distributor::HttpPackageResolver::getRepository(std::string_view packageDomain)
{
    const auto &ctx = m_priv->ctx;

    tempo_utils::Url repositoryUrl;
    TU_ASSIGN_OR_RETURN (repositoryUrl, resolveLocation(
        packageDomain, tempo_utils::UrlPath::fromString("repository.json")));

    TU_RETURN_IF_NOT_OK (performGet(repositoryUrl));

    tempo_config::ConfigNode rootNode;
    TU_ASSIGN_OR_RETURN (rootNode, tempo_config::read_config_string(ctx.body));

    auto rootMap = rootNode.toMap();
    auto repositoryMap = rootMap.mapAt("repository").toMap();

    RepositoryDescriptor repository;

    zuri_packager::PackageIdParser packageIdParser;
    RepositoryCollectionParser collectionParser;
    tempo_config::MapKVParser collectionsParser(&packageIdParser, &collectionParser);
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(repository.collections, collectionsParser,
        repositoryMap, "collections"));

    return repository;
}

class CollectionVersionParser
    : public tempo_config::AbstractConverter<zuri_distributor::CollectionDescriptor::Version> {
public:
    tempo_utils::Status convertValue(
        const tempo_config::ConfigNode &node,
        zuri_distributor::CollectionDescriptor::Version &value) const override
    {
        zuri_distributor::CollectionDescriptor::Version version;

        auto map = node.toMap();

        tempo_config::TimeParser uploadedAtParser(absl::RFC3339_full);
        absl::Time uploadedAt;
        TU_RETURN_IF_NOT_OK (tempo_config::parse_config(uploadedAt, uploadedAtParser,
            map, "uploadedAt"));
        version.uploadDateEpochMillis = absl::ToUnixMillis(uploadedAt);

        tempo_config::BooleanParser prunedParser(false);
        TU_RETURN_IF_NOT_OK (tempo_config::parse_config(version.pruned, prunedParser,
            map, "pruned"));

        value = version;
        return {};
    }
};

tempo_utils::Result<zuri_distributor::CollectionDescriptor>
zuri_distributor::HttpPackageResolver::getCollection(
    const zuri_packager::PackageId &packageId)
{
    const auto &ctx = m_priv->ctx;

    tempo_utils::Url collectionUrl;
    TU_ASSIGN_OR_RETURN (collectionUrl, resolveLocation(
        packageId.getDomain(), tempo_utils::UrlPath::fromString("collections")
            .traverse(tempo_utils::UrlPathPart(packageId.toString()))
            .traverse(tempo_utils::UrlPathPart("collection.json"))));

    TU_RETURN_IF_NOT_OK (performGet(collectionUrl));

    tempo_config::ConfigNode rootNode;
    TU_ASSIGN_OR_RETURN (rootNode, tempo_config::read_config_string(ctx.body));

    auto rootMap = rootNode.toMap();
    auto collectionMap = rootMap.mapAt("collection").toMap();

    CollectionDescriptor collection;
    collection.id = packageId;

    zuri_packager::PackageVersionParser packageVersionParser;
    CollectionVersionParser versionParser;
    tempo_config::MapKVParser versionsParser(&packageVersionParser, &versionParser);
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(collection.versions, versionsParser,
        collectionMap, "versions"));

    return collection;
}

tempo_utils::Result<zuri_distributor::PackageDescriptor>
zuri_distributor::HttpPackageResolver::getPackage(
    const zuri_packager::PackageId &packageId,
    const zuri_packager::PackageVersion &packageVersion)
{
    const auto &ctx = m_priv->ctx;

    tempo_utils::Url packageUrl;
    TU_ASSIGN_OR_RETURN (packageUrl, resolveLocation(
        packageId.getDomain(), tempo_utils::UrlPath::fromString("collections")
            .traverse(tempo_utils::UrlPathPart(packageId.toString()))
            .traverse(tempo_utils::UrlPathPart("versions"))
            .traverse(tempo_utils::UrlPathPart(packageVersion.toString()))
            .traverse(tempo_utils::UrlPathPart("package.json"))));

    TU_RETURN_IF_NOT_OK (performGet(packageUrl));

    tempo_config::ConfigNode rootNode;
    TU_ASSIGN_OR_RETURN (rootNode, tempo_config::read_config_string(ctx.body));

    auto rootMap = rootNode.toMap();
    auto packageMap = rootMap.mapAt("package").toMap();

    PackageDescriptor package;
    package.id = packageId;
    package.version = packageVersion;

    zuri_packager::PackageIdParser packageIdParser;
    zuri_packager::PackageVersionParser packageVersionParser;
    tempo_config::MapKVParser dependenciesParser(&packageIdParser, &packageVersionParser);
    absl::flat_hash_map<zuri_packager::PackageId, zuri_packager::PackageVersion> dependenciesMap;
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(dependenciesMap, dependenciesParser,
        packageMap, "dependencies"));
    for (const auto &dependency : dependenciesMap) {
        package.dependencies.insert(zuri_packager::PackageSpecifier(
            dependency.first, dependency.second));
    }

    tempo_config::UrlParser urlParser;
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(package.url, urlParser,
        packageMap, "url"));

    tempo_config::TimeParser uploadedAtParser(absl::RFC3339_full);
    absl::Time uploadedAt;
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(uploadedAt, uploadedAtParser,
        packageMap, "uploadedAt"));
    package.uploadDateEpochMillis = absl::ToUnixMillis(uploadedAt);

    tempo_config::BooleanParser prunedParser(false);
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(package.pruned, prunedParser,
        packageMap, "pruned"));

    return package;
}

tempo_utils::Result<std::shared_ptr<zuri_distributor::HttpPackageResolver>>
zuri_distributor::HttpPackageResolver::create(const HttpPackageResolverOptions &options)
{
    auto priv = std::make_unique<Priv>();
    priv->ctx.handle = curl_easy_init();
    curl_easy_setopt(priv->ctx.handle, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(priv->ctx.handle, CURLOPT_WRITEDATA, &priv->ctx);

    return std::shared_ptr<HttpPackageResolver>(new HttpPackageResolver(std::move(priv)));
}
