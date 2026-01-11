#ifndef ZURI_TOOLING_ENVIRONMENT_H
#define ZURI_TOOLING_ENVIRONMENT_H

#include <filesystem>

#include <tempo_utils/result.h>

namespace zuri_tooling {

    /**
     *
     */
    constexpr const char * const kEnvironmentDatabaseName = "environment.db";

    class Environment {
    public:
        Environment();
        Environment(const Environment &other);

        bool isValid() const;

        std::filesystem::path getEnvironmentDatabaseFile() const;
        std::filesystem::path getEnvironmentDirectory() const;
        std::filesystem::path getBinDirectory() const;
        std::filesystem::path getLibDirectory() const;
        std::filesystem::path getPackagesDirectory() const;
        std::filesystem::path getConfigDirectory() const;

        static tempo_utils::Result<Environment> openOrCreate(const std::filesystem::path &environmentDirectory);
        static tempo_utils::Result<Environment> open(const std::filesystem::path &environmentDirectoryOrDatabaseFile);
        static tempo_utils::Result<Environment> find(const std::filesystem::path &searchStart = {});
        static tempo_utils::Result<Environment> relativeTo(const std::filesystem::path &programLocation);

    private:
        struct Priv {
            std::filesystem::path environmentDatabaseFile;
            std::filesystem::path environmentDirectory;
            std::filesystem::path binDirectory;
            std::filesystem::path libDirectory;
            std::filesystem::path packagesDirectory;
            std::filesystem::path configDirectory;
        };
        std::shared_ptr<Priv> m_priv;

        Environment(
            const std::filesystem::path &environmentDatabaseFile,
            const std::filesystem::path &environmentDirectory,
            const std::filesystem::path &binDirectory,
            const std::filesystem::path &libDirectory,
            const std::filesystem::path &packagesDirectory,
            const std::filesystem::path &configDirectory);
    };
}

#endif // ZURI_TOOLING_ENVIRONMENT_H