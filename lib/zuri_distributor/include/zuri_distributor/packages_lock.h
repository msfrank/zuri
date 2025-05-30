#ifndef ZURI_DISTRIBUTOR_PACKAGES_LOCK_H
#define ZURI_DISTRIBUTOR_PACKAGES_LOCK_H

#include <absl/container/btree_map.h>
#include <tempo_utils/result.h>

#include "distributor_result.h"
#include "zuri_packager/package_specifier.h"

namespace zuri_distributor {

    struct LockEntry {
        zuri_packager::PackageSpecifier specifier;
        tu_int64 lastCheckedTimeInMs = 0;
    };

    class PackagesLock {
    public:
        static tempo_utils::Result<std::shared_ptr<PackagesLock>> openOrCreate(
            const std::filesystem::path &packagesLockPath);

        bool hasEntry(const std::string &packageId) const;
        LockEntry getEntry(const std::string &packageId) const;
        tempo_utils::Status updateEntry(const zuri_packager::PackageSpecifier &specifier, tu_int64 checkTimeInMs);
        tempo_utils::Status writeAndClose();

    private:
        absl::btree_map<std::string,LockEntry> m_lockEntries;

        PackagesLock(absl::btree_map<std::string,LockEntry> &&m_lockEntries);
    };
}

#endif // ZURI_DISTRIBUTOR_PACKAGES_LOCK_H
