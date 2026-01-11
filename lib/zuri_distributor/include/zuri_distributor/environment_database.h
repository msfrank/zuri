#ifndef ZURI_DISTRIBUTOR_ENVIRONMENT_DATABASE_H
#define ZURI_DISTRIBUTOR_ENVIRONMENT_DATABASE_H

#include <filesystem>

#include <sqlite3.h>

#include <tempo_utils/result.h>

namespace zuri_distributor {

    class EnvironmentDatabase {
    public:
        ~EnvironmentDatabase();

        static tempo_utils::Result<std::shared_ptr<EnvironmentDatabase>> openOrCreate(
            const std::filesystem::path &databaseFilePath);
        static tempo_utils::Result<std::shared_ptr<EnvironmentDatabase>> open(
            const std::filesystem::path &databaseFilePath);

        std::filesystem::path getDatabaseFilePath() const;

    private:
        std::filesystem::path m_databaseFilePath;
        sqlite3 *m_db;

        sqlite3_stmt *m_listSpecifiers = nullptr;
        sqlite3_stmt *m_insertSpecifier = nullptr;

        static tempo_utils::Result<std::shared_ptr<EnvironmentDatabase>> open(
            const std::filesystem::path &databaseFilePath,
            int flags);

        EnvironmentDatabase(
            const std::filesystem::path &lockFilePath,
            sqlite3 *db);

        tempo_utils::Status prepare();
    };
}

#endif // ZURI_DISTRIBUTOR_ENVIRONMENT_DATABASE_H