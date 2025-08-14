
#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/parse_config.h>
#include <zuri_tooling/package_store.h>
#include <zuri_tooling/tooling_conversions.h>

zuri_tooling::PackageStore::PackageStore(
    const tempo_config::ConfigMap &dcacheMap,
    const tempo_config::ConfigMap &ucacheMap,
    const tempo_config::ConfigMap &icacheMap,
    const tempo_config::ConfigMap &tcacheMap)
    : m_dcacheMap(dcacheMap),
      m_ucacheMap(ucacheMap),
      m_icacheMap(icacheMap),
      m_tcacheMap(tcacheMap)
{
}

tempo_utils::Status
zuri_tooling::PackageStore::configure()
{
    tempo_config::StringParser importNameParser;
    PackageCacheEntryParser packageCacheEntryParser;
    tempo_config::SharedPtrConstTParser sharedConstImportEntryParser(&packageCacheEntryParser);

    if (m_dcacheMap.getNodeType() == tempo_config::ConfigNodeType::kMap) {
        TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
            m_dcache, sharedConstImportEntryParser, m_dcacheMap));
    }
    if (m_ucacheMap.getNodeType() == tempo_config::ConfigNodeType::kMap) {
        TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
            m_ucache, sharedConstImportEntryParser, m_ucacheMap));
    }
    if (m_icacheMap.getNodeType() == tempo_config::ConfigNodeType::kMap) {
        TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
            m_icache, sharedConstImportEntryParser, m_icacheMap));
    }
    if (m_tcacheMap.getNodeType() == tempo_config::ConfigNodeType::kMap) {
        TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
            m_tcache, sharedConstImportEntryParser, m_tcacheMap));
    }

    return {};
}

bool
zuri_tooling::PackageStore::hasDCache() const
{
    return m_dcache != nullptr;
}

std::shared_ptr<const zuri_tooling::PackageCacheEntry>
zuri_tooling::PackageStore::getDCache() const
{
    return m_dcache;
}

bool
zuri_tooling::PackageStore::hasUCache() const
{
    return m_ucache != nullptr;
}

std::shared_ptr<const zuri_tooling::PackageCacheEntry>
zuri_tooling::PackageStore::getUCache() const
{
    return m_ucache;
}

bool
zuri_tooling::PackageStore::hasICache() const
{
    return m_icache != nullptr;
}

std::shared_ptr<const zuri_tooling::PackageCacheEntry>
zuri_tooling::PackageStore::getICache() const
{
    return m_icache;
}

bool
zuri_tooling::PackageStore::hasTCache() const
{
    return m_tcache != nullptr;
}

std::shared_ptr<const zuri_tooling::PackageCacheEntry>
zuri_tooling::PackageStore::getTCache() const
{
    return m_tcache;
}
