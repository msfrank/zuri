#ifndef ZURI_TOOLING_RESOLVER_CONFIG_H
#define ZURI_TOOLING_RESOLVER_CONFIG_H

#include <tempo_config/config_types.h>
#include <tempo_utils/status.h>
#include <tempo_utils/url.h>

namespace zuri_tooling {

    class ResolverConfig {
    public:
        explicit ResolverConfig(const tempo_config::ConfigMap &resolverMap);

        tempo_utils::Status configure();

        std::filesystem::path getHttpRootCABundleFile() const;
        tempo_utils::Url getHttpProxyUrl() const;
        std::string getHttpProxyUsername() const;
        std::string getHttpProxyPassword() const;

    private:
        tempo_config::ConfigMap m_resolverMap;

        std::filesystem::path m_httpRootCABundleFile;
        tempo_utils::Url m_httpProxyUrl;
        std::string m_httpProxyUsername;
        std::string m_httpProxyPassword;
    };
}

#endif //ZURI_TOOLING_RESOLVER_CONFIG_H