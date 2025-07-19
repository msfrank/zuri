
#include <curl/curl.h>

#include <tempo_utils/file_appender.h>
#include <zuri_distributor/distributor_result.h>
#include <zuri_distributor/package_fetcher.h>

struct Manager;

/**
 *
 */
struct Fetch {
    // config
    zuri_packager::PackageSpecifier specifier;
    tempo_utils::Url url;
    Manager *manager = nullptr;

    // state
    CURL *handle = nullptr;
    curl_off_t bytesExpected = 0;
    curl_off_t bytesFetched = 0;
    std::unique_ptr<tempo_utils::FileAppender> appender;
    tempo_utils::Status status;

    ~Fetch() {
        if (handle != nullptr) {
            curl_easy_cleanup(handle);
        }
    }
};

/**
 *
 */
struct Manager {
    // config
    std::filesystem::path downloadRoot;
    int pollTimeoutInMs = 100;

    // state
    CURLM *multi = nullptr;
    absl::flat_hash_map<zuri_packager::PackageSpecifier,std::unique_ptr<Fetch>> fetches;
    curl_off_t totalBytesExpected = 0;
    curl_off_t totalBytesFetched = 0;

    ~Manager() {
        // free each fetch before cleaning up the multi handle
        for (auto &entry : fetches) {
            auto &fetch = entry.second;
            if (fetch != nullptr) {
                auto ret = curl_multi_remove_handle(multi, fetch->handle);
                TU_LOG_WARN_IF (ret != CURLM_OK) << "curl_multi_remove_handle failed: {}", curl_multi_strerror(ret);
                fetch.reset();
            }
        }
        if (multi != nullptr) {
            auto ret = curl_multi_cleanup(multi);
            TU_LOG_WARN_IF (ret != CURLM_OK) << "curl_multi_cleanup failed: " << curl_multi_strerror(ret);
        }
    }
};

struct zuri_distributor::PackageFetcher::Priv {
    Manager manager;
};

static size_t
header_callback(char *buffer, size_t size, size_t nitems, void *userdata)
{
    auto *fetch = (Fetch *) userdata;

    long response_code;
    curl_easy_getinfo(fetch->handle, CURLINFO_RESPONSE_CODE, &response_code);
    if (response_code >= 300) {
        fetch->status = zuri_distributor::DistributorStatus::forCondition(
            zuri_distributor::DistributorCondition::kDistributorInvariant,
            "encountered {} status code during fetch", response_code);
        return CURL_WRITEFUNC_ERROR;
    }

    return nitems;
}

static size_t
write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    auto *fetch = (Fetch *) userdata;

    // if status was set then signal error
    if (fetch->status.notOk())
        return CURL_WRITEFUNC_ERROR;

    // if appender doesn't exist then create one
    if (fetch->appender == nullptr) {
        auto packagePath = fetch->specifier.toPackagePath(fetch->manager->downloadRoot);
        fetch->appender = std::make_unique<tempo_utils::FileAppender>(
            packagePath, tempo_utils::FileAppenderMode::CREATE_OR_OVERWRITE);
        if (!fetch->appender->isValid()) {
            fetch->status = fetch->appender->getStatus();
            fetch->appender.reset();
            return CURL_WRITEFUNC_ERROR;
        }
    }

    // write bytes to the appender
    std::string_view bytes(ptr, nmemb);
    auto status = fetch->appender->appendBytes(bytes);
    if (status.notOk()) {
        fetch->status = status;
        fetch->appender.reset();
        return CURL_WRITEFUNC_ERROR;
    }

    return nmemb;
}

static int
progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    auto *fetch = (Fetch *) clientp;
    auto *manager = fetch->manager;

    auto additionalFetched = dlnow - fetch->bytesFetched;
    if (additionalFetched > 0) {
        fetch->bytesFetched += additionalFetched;
        manager->totalBytesFetched += additionalFetched;
    }

    auto additionalExpected = dltotal - fetch->bytesExpected;
    if (additionalExpected > 0) {
        fetch->bytesExpected += dltotal;
        manager->totalBytesExpected += dltotal;
    }

    return 0;
}

zuri_distributor::PackageFetcher::PackageFetcher(const PackageFetcherOptions &options)
    : m_options(options)
{
}

// destructor needs to be defined in implementation in order for pImpl to work
zuri_distributor::PackageFetcher::~PackageFetcher()
{
}

tempo_utils::Status
zuri_distributor::PackageFetcher::configure()
{
    if (m_priv != nullptr)
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "package fetcher is already configured");

    auto priv = std::make_unique<Priv>();
    priv->manager.pollTimeoutInMs = m_options.pollTimeoutInMs;

    // set the download root if given, otherwise default to the current directory
    if (!m_options.downloadRoot.empty()) {
        priv->manager.downloadRoot = m_options.downloadRoot;
    } else {
        priv->manager.downloadRoot = std::filesystem::current_path();
    }
    if (!is_directory(priv->manager.downloadRoot))
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "download root {} is not a valid directory", priv->manager.downloadRoot.string());

    // allocate the multi handle
    priv->manager.multi = curl_multi_init();

    // configuration succeeded
    m_priv = std::move(priv);
    return {};
}

tempo_utils::Status
zuri_distributor::PackageFetcher::addPackage(
    const zuri_packager::PackageSpecifier &specifier,
    const tempo_utils::Url &url)
{
    if (m_priv == nullptr)
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "package fetcher is not configured");

    switch (url.getKnownScheme()) {
        case tempo_utils::KnownUrlScheme::File:
        case tempo_utils::KnownUrlScheme::Http:
        case tempo_utils::KnownUrlScheme::Https:
            break;
        default:
            return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
                "invalid url scheme '{}' for package {}", url.schemeView(), specifier.toString());
    }

    if (m_priv->manager.fetches.contains(specifier))
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "package {} is already requested", specifier.toString());

    auto fetch = std::make_unique<Fetch>();
    fetch->specifier = specifier;
    fetch->url = url;
    fetch->manager = &m_priv->manager;

    fetch->handle = curl_easy_init();
    curl_easy_setopt(fetch->handle, CURLOPT_PRIVATE, fetch.get());
    curl_easy_setopt(fetch->handle, CURLOPT_URL, url.uriView());

    // configure callbacks
    curl_easy_setopt(fetch->handle, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(fetch->handle, CURLOPT_HEADERDATA, fetch.get());
    curl_easy_setopt(fetch->handle, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(fetch->handle, CURLOPT_WRITEDATA, fetch.get());
    curl_easy_setopt(fetch->handle, CURLOPT_NOPROGRESS, 0);
    curl_easy_setopt(fetch->handle, CURLOPT_XFERINFOFUNCTION, progress_callback);
    curl_easy_setopt(fetch->handle, CURLOPT_XFERINFODATA, fetch.get());

    auto ret = curl_multi_add_handle(m_priv->manager.multi, fetch->handle);
    if (ret != CURLM_OK)
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "curl_multi_add_handle failed: {}", curl_multi_strerror(ret));

    m_priv->manager.fetches[specifier] = std::move(fetch);

    return {};
}

inline tempo_utils::Status
poll_until_complete(Manager &manager)
{
    int stillRunning;
    do {
        CURLMcode ret = curl_multi_perform(manager.multi, &stillRunning);
        if (ret != CURLM_OK)
            return zuri_distributor::DistributorStatus::forCondition(
                zuri_distributor::DistributorCondition::kDistributorInvariant,
                "curl_multi_perform failed: {}", curl_multi_strerror(ret));
        if (stillRunning) {
            ret = curl_multi_poll(manager.multi, nullptr, 0, manager.pollTimeoutInMs, nullptr);
            if (ret != CURLM_OK)
                return zuri_distributor::DistributorStatus::forCondition(
                    zuri_distributor::DistributorCondition::kDistributorInvariant,
                    "curl_multi_poll failed: {}", curl_multi_strerror(ret));
        }
    } while (stillRunning);
    return {};
}

tempo_utils::Status
zuri_distributor::PackageFetcher::fetchPackages()
{
    if (m_priv == nullptr)
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "package fetcher is not configured");

    auto pollStatus = poll_until_complete(m_priv->manager);

    CURLMsg *msg;
    int msgsLeft;

    /* See how the transfers went */
    while((msg = curl_multi_info_read(m_priv->manager.multi, &msgsLeft)) != nullptr) {
        if(msg->msg != CURLMSG_DONE)
            continue;

        char *userdata;
        curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &userdata);
        auto *fetchPtr = (Fetch *) userdata;
        if (fetchPtr == nullptr)
            continue;

        FetchResult result;
        if (fetchPtr->status.isOk()) {
            result.path = fetchPtr->appender->getAbsolutePath();
        } else {
            result.status = fetchPtr->status;
        }

        m_results[fetchPtr->specifier] = std::move(result);
    }

    return {};
}

bool
zuri_distributor::PackageFetcher::hasResult(const zuri_packager::PackageSpecifier &specifier) const
{
    return m_results.contains(specifier);
}

zuri_distributor::FetchResult
zuri_distributor::PackageFetcher::getResult(const zuri_packager::PackageSpecifier &specifier) const
{
    auto entry = m_results.find(specifier);
    if (entry != m_results.cend())
        return entry->second;
    return {};
}

absl::flat_hash_map<zuri_packager::PackageSpecifier,zuri_distributor::FetchResult>::const_iterator
zuri_distributor::PackageFetcher::resultsBegin() const
{
    return m_results.cbegin();
}

absl::flat_hash_map<zuri_packager::PackageSpecifier,zuri_distributor::FetchResult>::const_iterator
zuri_distributor::PackageFetcher::resultsEnd() const
{
    return m_results.cend();
}

int
zuri_distributor::PackageFetcher::numResults() const
{
    return m_results.size();
}