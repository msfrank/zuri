#ifndef ZURI_DISTRIBUTOR_PACKAGE_DATABASE_H
#define ZURI_DISTRIBUTOR_PACKAGE_DATABASE_H

#include <filesystem>

#include <sqlite3.h>

#include <tempo_utils/result.h>

namespace zuri_distributor {

    class PackageDatabase {
    public:
        ~PackageDatabase();

        static tempo_utils::Result<std::shared_ptr<PackageDatabase>> openOrCreate(
            const std::filesystem::path &databaseFilePath);
        static tempo_utils::Result<std::shared_ptr<PackageDatabase>> open(
            const std::filesystem::path &databaseFilePath);

        std::filesystem::path getDatabaseFilePath() const;

    private:
        std::filesystem::path m_databaseFilePath;
        sqlite3 *m_db;

        sqlite3_stmt *m_listSpecifiers = nullptr;
        sqlite3_stmt *m_insertSpecifier = nullptr;

        static tempo_utils::Result<std::shared_ptr<PackageDatabase>> open(
            const std::filesystem::path &databaseFilePath,
            int flags);

        PackageDatabase(
            const std::filesystem::path &databaseFilePath,
            sqlite3 *db);

        tempo_utils::Status prepare();
    };
}

#endif // ZURI_DISTRIBUTOR_PACKAGE_DATABASE_H