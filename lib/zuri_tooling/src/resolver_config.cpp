
#include <tempo_config/base_conversions.h>
#include <tempo_config/parse_config.h>
#include <zuri_tooling/tooling_conversions.h>
#include <zuri_tooling/resolver_config.h>

zuri_tooling::ResolverConfig::ResolverConfig(const tempo_config::ConfigMap &resolverMap)
    : m_resolverMap(resolverMap)
{
}

tempo_utils::Status
zuri_tooling::ResolverConfig::configure()
{
    // determine http root CA bundle file
    tempo_config::PathParser httpRootCABundleFileParser(std::filesystem::path{});
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(m_httpRootCABundleFile, httpRootCABundleFileParser,
        m_resolverMap, "httpRootCABundleFile"));

    // determine http proxy url
    tempo_config::UrlParser httpProxyUrlParser(tempo_utils::Url{});
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(m_httpProxyUrl, httpProxyUrlParser,
        m_resolverMap, "httpProxyUrl"));

    // determine http proxy username
    tempo_config::StringParser httpProxyUsernameParser(std::string{});
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(m_httpProxyUsername, httpProxyUsernameParser,
        m_resolverMap, "httpProxyUsername"));

    // determine http proxy password
    tempo_config::StringParser httpProxyPasswordParser(std::string{});
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(m_httpProxyPassword, httpProxyPasswordParser,
        m_resolverMap, "httpProxyPassword"));

    return {};
}

std::filesystem::path
zuri_tooling::ResolverConfig::getHttpRootCABundleFile() const
{
    return m_httpRootCABundleFile;
}

tempo_utils::Url
zuri_tooling::ResolverConfig::getHttpProxyUrl() const
{
    return m_httpProxyUrl;
}

std::string
zuri_tooling::ResolverConfig::getHttpProxyUsername() const
{
    return m_httpProxyUsername;
}

std::string
zuri_tooling::ResolverConfig::getHttpProxyPassword() const
{
    return m_httpProxyPassword;
}