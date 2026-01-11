#ifndef ZURI_TOOLING_PROJECT_H
#define ZURI_TOOLING_PROJECT_H

#include <filesystem>

#include <tempo_config/config_types.h>
#include <tempo_utils/result.h>

#include "environment.h"

namespace zuri_tooling {

    /**
     *
     */
    constexpr const char * const kProjectConfigName = "project.config";

    /**
     *
     */
    constexpr const char * const kProjectEnvironmentDirectoryName = ".zurienv";

    /**
     *
     */
    constexpr const char * const kProjectBuildDirectoryName = ".zuribuild";

    class Project {
    public:
        Project();
        Project(const Project &other);

        bool isValid() const;

        std::filesystem::path getProjectConfigFile() const;
        std::filesystem::path getProjectDirectory() const;
        std::filesystem::path getSrcDirectory() const;
        std::filesystem::path getConfigDirectory() const;
        std::filesystem::path getBuildDirectory() const;
        std::filesystem::path getEnvironmentDirectory() const;

        static tempo_utils::Result<Project> openOrCreate(
            const std::filesystem::path &projectDirectory,
            const tempo_config::ConfigMap &projectMap = {});
        static tempo_utils::Result<Project> open(const std::filesystem::path &projectDirectoryOrConfigFile);
        static tempo_utils::Result<Project> find(const std::filesystem::path &searchStart = {});

    private:
        struct Priv {
            std::filesystem::path projectConfigFile;
            std::filesystem::path projectDirectory;
            std::filesystem::path srcDirectory;
            std::filesystem::path configDirectory;
            std::filesystem::path buildDirectory;
            std::filesystem::path environmentDirectory;
        };
        std::shared_ptr<Priv> m_priv;

        Project(
            const std::filesystem::path &projectConfigFile,
            const std::filesystem::path &projectDirectory,
            const std::filesystem::path &srcDirectory,
            const std::filesystem::path &configDirectory,
            const std::filesystem::path &buildDirectory,
            const std::filesystem::path &environmentDirectory);
    };
}

#endif // ZURI_TOOLING_PROJECT_H