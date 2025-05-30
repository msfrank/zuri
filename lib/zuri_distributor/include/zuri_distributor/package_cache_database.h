#ifndef ZURI_DISTRIBUTOR_PACKAGE_CACHE_DATABASE_H
#define ZURI_DISTRIBUTOR_PACKAGE_CACHE_DATABASE_H

#include <filesystem>

#include <sqlite3.h>

#include <tempo_utils/result.h>

namespace zuri_distributor {

    class PackageCacheDatabase {
    public:
        ~PackageCacheDatabase();

        static tempo_utils::Result<std::shared_ptr<PackageCacheDatabase>> open(
            const std::filesystem::path &databaseFilePath);

    private:
        std::filesystem::path m_databaseFilePath;
        sqlite3 *m_db;

        PackageCacheDatabase(
            const std::filesystem::path &lockFilePath,
            sqlite3 *db);
    };
}

#endif // ZURI_DISTRIBUTOR_PACKAGE_CACHE_DATABASE_H
