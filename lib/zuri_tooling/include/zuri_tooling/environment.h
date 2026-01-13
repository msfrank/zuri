#ifndef ZURI_TOOLING_ENVIRONMENT_H
#define ZURI_TOOLING_ENVIRONMENT_H

#include <filesystem>

#include <tempo_utils/result.h>

#include "distribution.h"

namespace zuri_tooling {

    /**
     *
     */
    constexpr const char * const kEnvironmentConfigName = "environment.config";

    /**
     *
     */
    struct EnvironmentOpenOrCreateOptions {
        /**
         * if true then project must not exist, and will be created.
         */
        bool exclusive = false;
        /**
         *
         */
        Distribution distribution = {};
        /**
         *
         */
        std::vector<std::filesystem::path> extraLibDirs = {};
        /**
         * map contents will be written to environment.config
         */
        tempo_config::ConfigMap projectMap = {};
    };

    /**
     *
     */
    class Environment {
    public:
        Environment();
        Environment(const Environment &other);

        bool isValid() const;

        std::filesystem::path getEnvironmentConfigFile() const;
        std::filesystem::path getEnvironmentDirectory() const;
        std::filesystem::path getBinDirectory() const;
        std::filesystem::path getLibDirectory() const;
        std::filesystem::path getPackagesDirectory() const;
        std::filesystem::path getPackagesDatabaseFile() const;
        std::filesystem::path getConfigDirectory() const;

        static tempo_utils::Result<Environment> openOrCreate(
            const std::filesystem::path &environmentDirectory,
            const EnvironmentOpenOrCreateOptions &options = {});
        static tempo_utils::Result<Environment> open(const std::filesystem::path &environmentDirectoryOrConfigFile);
        static tempo_utils::Result<Environment> find(const std::filesystem::path &searchStart = {});

    private:
        struct Priv {
            std::filesystem::path environmentConfigFile;
            std::filesystem::path environmentDirectory;
            std::filesystem::path binDirectory;
            std::filesystem::path libDirectory;
            std::filesystem::path packagesDirectory;
            std::filesystem::path packagesDatabaseFile;
            std::filesystem::path configDirectory;
        };
        std::shared_ptr<Priv> m_priv;

        Environment(
            const std::filesystem::path &environmentConfigFile,
            const std::filesystem::path &environmentDirectory,
            const std::filesystem::path &binDirectory,
            const std::filesystem::path &libDirectory,
            const std::filesystem::path &packagesDirectory,
            const std::filesystem::path &packagesDatabaseFile,
            const std::filesystem::path &configDirectory);
    };
}

#endif // ZURI_TOOLING_ENVIRONMENT_H