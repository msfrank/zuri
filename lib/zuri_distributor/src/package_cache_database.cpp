
#include <zuri_distributor/distributor_result.h>
#include <zuri_distributor/package_cache_database.h>

zuri_distributor::PackageCacheDatabase::PackageCacheDatabase(
    const std::filesystem::path &databaseFilePath,
    sqlite3 *db)
    : m_databaseFilePath(databaseFilePath),
      m_db(db)
{
    TU_ASSERT (!m_databaseFilePath.empty());
    TU_ASSERT (m_db != nullptr);
}

zuri_distributor::PackageCacheDatabase::~PackageCacheDatabase()
{
    auto ret = sqlite3_close_v2(m_db);
    TU_LOG_WARN_IF (ret != SQLITE_OK) << "sqlite close failed unexpectedly: " << sqlite3_errstr(ret);
}

tempo_utils::Result<std::shared_ptr<zuri_distributor::PackageCacheDatabase>>
zuri_distributor::PackageCacheDatabase::open(const std::filesystem::path &databaseFilePath)
{
    sqlite3 *db = nullptr;
    int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_NOFOLLOW;
    auto ret = sqlite3_open_v2(databaseFilePath.c_str(), &db, flags, nullptr);
    if (ret != SQLITE_OK)
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "failed to open package cache database {}: {}",
            databaseFilePath.string(), sqlite3_errstr(ret));

    return std::shared_ptr<PackageCacheDatabase>(new PackageCacheDatabase(databaseFilePath, db));
}
