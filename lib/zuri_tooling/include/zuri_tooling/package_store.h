// #ifndef ZURI_TOOLING_PACKAGE_STORE_H
// #define ZURI_TOOLING_PACKAGE_STORE_H
//
// #include <lyric_build/build_types.h>
// #include <tempo_utils/status.h>
//
// namespace zuri_tooling {
//
//     struct PackageCacheEntry {
//         bool writeable;
//     };
//
//     class PackageStore {
//     public:
//         PackageStore(
//             const tempo_config::ConfigMap &dcacheMap,
//             const tempo_config::ConfigMap &ucacheMap,
//             const tempo_config::ConfigMap &icacheMap,
//             const tempo_config::ConfigMap &tcacheMap);
//
//         tempo_utils::Status configure();
//
//         bool hasDCache() const;
//         std::shared_ptr<const PackageCacheEntry> getDCache() const;
//         bool hasUCache() const;
//         std::shared_ptr<const PackageCacheEntry> getUCache() const;
//         bool hasICache() const;
//         std::shared_ptr<const PackageCacheEntry> getICache() const;
//         bool hasTCache() const;
//         std::shared_ptr<const PackageCacheEntry> getTCache() const;
//
//     private:
//         tempo_config::ConfigMap m_dcacheMap;
//         tempo_config::ConfigMap m_ucacheMap;
//         tempo_config::ConfigMap m_icacheMap;
//         tempo_config::ConfigMap m_tcacheMap;
//
//         std::shared_ptr<const PackageCacheEntry> m_dcache;
//         std::shared_ptr<const PackageCacheEntry> m_ucache;
//         std::shared_ptr<const PackageCacheEntry> m_icache;
//         std::shared_ptr<const PackageCacheEntry> m_tcache;
//     };
// }
//
// #endif // ZURI_TOOLING_PACKAGE_STORE_H