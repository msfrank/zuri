#ifndef ZURI_DISTRIBUTOR_PACKAGE_DATABASE_H
#define ZURI_DISTRIBUTOR_PACKAGE_DATABASE_H

#include <filesystem>

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

        struct Priv;
        std::unique_ptr<Priv> m_priv;

        static tempo_utils::Result<std::shared_ptr<PackageDatabase>> open(
            const std::filesystem::path &databaseFilePath,
            int flags);

        PackageDatabase(
            const std::filesystem::path &databaseFilePath,
            std::unique_ptr<Priv> priv);

        tempo_utils::Status prepare();
    };
}

#endif // ZURI_DISTRIBUTOR_PACKAGE_DATABASE_H