#ifndef ZURI_TOOLING_PROJECT_H
#define ZURI_TOOLING_PROJECT_H

#include <filesystem>

#include <tempo_config/config_types.h>
#include <tempo_utils/result.h>

#include "distribution.h"

namespace zuri_tooling {

    /**
     * The file name of the project config file within a project.
     */
    constexpr const char * const kProjectConfigName = "project.config";

    /**
     * The directory name of the environment directory within a project.
     */
    constexpr const char * const kProjectBuildDirectoryName = ".zuribuild";

    /**
     * Options structure which controls the creation of a new Project.
     */
    struct ProjectOpenOrCreateOptions {
        bool exclusive = false;
        Distribution distribution = {};
        std::vector<std::filesystem::path> extraLibDirs = {};
        tempo_config::ConfigMap projectMap = {};
        bool linked = false;
        std::filesystem::path projectConfigTarget = {};
    };

    /**
     * A Project is a directory structure containing the files and directories needed to build targets.
     */
    class Project {
    public:
        Project();
        Project(const Project &other);

        bool isValid() const;
        bool isLinked() const;

        std::filesystem::path getProjectConfigFile() const;
        std::filesystem::path getProjectDirectory() const;
        std::filesystem::path getConfigDirectory() const;
        std::filesystem::path getTargetsDirectory() const;
        std::filesystem::path getBuildDirectory() const;
        std::filesystem::path getBuildEnvironmentDirectory() const;

        static tempo_utils::Result<Project> openOrCreate(
            const std::filesystem::path &projectDirectory,
            const ProjectOpenOrCreateOptions &options = {});
        static tempo_utils::Result<Project> open(const std::filesystem::path &projectDirectoryOrConfigFile);
        static tempo_utils::Result<Project> find(const std::filesystem::path &searchStart = {});

    private:
        struct Priv {
            std::filesystem::path projectConfigFile;
            std::filesystem::path projectDirectory;
            std::filesystem::path configDirectory;
            std::filesystem::path targetsDirectory;
            std::filesystem::path buildDirectory;
            std::filesystem::path buildEnvironmentDirectory;
        };
        std::shared_ptr<Priv> m_priv;

        Project(
            const std::filesystem::path &projectConfigFile,
            const std::filesystem::path &projectDirectory,
            const std::filesystem::path &configDirectory,
            const std::filesystem::path &targetsDirectory,
            const std::filesystem::path &buildDirectory,
            const std::filesystem::path &buildEnvironmentDirectory);
    };
}

#endif // ZURI_TOOLING_PROJECT_H