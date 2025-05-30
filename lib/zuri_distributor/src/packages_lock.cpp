
#include <zuri_distributor/packages_lock.h>

zuri_distributor::PackagesLock::PackagesLock(absl::btree_map<std::string,LockEntry> &&lockEntries)
    : m_lockEntries(std::move(lockEntries))
{
}

tempo_utils::Result<std::shared_ptr<zuri_distributor::PackagesLock>>
zuri_distributor::PackagesLock::openOrCreate(const std::filesystem::path &packagesLockPath)
{
    absl::btree_map<std::string,LockEntry> lockEntries;

    return std::shared_ptr<PackagesLock>(new PackagesLock(std::move(lockEntries)));
}

bool
zuri_distributor::PackagesLock::hasEntry(const std::string &packageId) const
{
    return m_lockEntries.contains(packageId);
}

zuri_distributor::LockEntry
zuri_distributor::PackagesLock::getEntry(const std::string &packageId) const
{
    auto entry = m_lockEntries.find(packageId);
    if (entry != m_lockEntries.cend())
        return entry->second;
    return {};
}

tempo_utils::Status
zuri_distributor::PackagesLock::updateEntry(const zuri_packager::PackageSpecifier &specifier, tu_int64 checkTimeInMs)
{
    return {};
}

tempo_utils::Status
writeAndClose()
{
    return {};
}
