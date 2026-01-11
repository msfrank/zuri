
#include <tempo_config/config_utils.h>
#include <zuri_tooling/project.h>
#include <zuri_tooling/tooling_result.h>

#include "zuri_tooling/environment.h"

zuri_tooling::Project::Project()
{
}

zuri_tooling::Project::Project(
    const std::filesystem::path &projectConfigFile,
    const std::filesystem::path &projectDirectory,
    const std::filesystem::path &srcDirectory,
    const std::filesystem::path &configDirectory,
    const std::filesystem::path &buildDirectory,
    const std::filesystem::path &environmentDirectory)
    : m_priv(std::make_shared<Priv>(
        projectConfigFile,
        projectDirectory,
        srcDirectory,
        configDirectory,
        buildDirectory,
        environmentDirectory))
{
    TU_ASSERT (!m_priv->projectConfigFile.empty());
    TU_ASSERT (!m_priv->projectDirectory.empty());
    TU_ASSERT (!m_priv->srcDirectory.empty());
    TU_ASSERT (!m_priv->configDirectory.empty());
    TU_ASSERT (!m_priv->buildDirectory.empty());
    TU_ASSERT (!m_priv->environmentDirectory.empty());
}

zuri_tooling::Project::Project(const Project &other)
    : m_priv(other.m_priv)
{
}

bool
zuri_tooling::Project::isValid() const
{
    return m_priv != nullptr;
}

std::filesystem::path
zuri_tooling::Project::getProjectConfigFile() const
{
    if (m_priv)
        return m_priv->projectConfigFile;
    return {};
}

std::filesystem::path
zuri_tooling::Project::getProjectDirectory() const
{
    if (m_priv)
        return m_priv->projectDirectory;
    return {};
}

std::filesystem::path
zuri_tooling::Project::getSrcDirectory() const
{
    if (m_priv)
        return m_priv->srcDirectory;
    return {};
}

std::filesystem::path
zuri_tooling::Project::getConfigDirectory() const
{
    if (m_priv)
        return m_priv->configDirectory;
    return {};
}

std::filesystem::path
zuri_tooling::Project::getBuildDirectory() const
{
    if (m_priv)
        return m_priv->buildDirectory;
    return {};
}

std::filesystem::path
zuri_tooling::Project::getEnvironmentDirectory() const
{
    if (m_priv)
        return m_priv->environmentDirectory;
    return {};
}

tempo_utils::Result<zuri_tooling::Project>
zuri_tooling::Project::openOrCreate(
    const std::filesystem::path &projectDirectory,
    const tempo_config::ConfigMap &projectMap)
{
    if (std::filesystem::exists(projectDirectory))
        return open(projectDirectory);

    std::error_code ec;
    std::filesystem::create_directory(projectDirectory, ec);
    if (ec)
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "failed to create zuri project directory {}", projectDirectory.string());

    auto srcDirectory = projectDirectory / "src";
    std::filesystem::create_directory(srcDirectory, ec);
    if (ec)
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "failed to create project src directory {}", srcDirectory.string());

    auto configDirectory = projectDirectory / "config";
    std::filesystem::create_directory(configDirectory, ec);
    if (ec)
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "failed to create project config directory {}", configDirectory.string());

    auto buildDirectory = projectDirectory / "build";
    std::filesystem::create_directory(buildDirectory, ec);
    if (ec)
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "failed to create project build directory {}", buildDirectory.string());

    auto environmentDirectory = projectDirectory / kProjectEnvironmentDirectoryName;
    Environment environment;
    TU_ASSIGN_OR_RETURN (environment, Environment::openOrCreate(environmentDirectory));

    auto projectConfigFile = projectDirectory / kProjectConfigName;
    TU_RETURN_IF_NOT_OK (tempo_config::write_config_file(projectMap, projectConfigFile));

    return Project(projectConfigFile, projectDirectory, srcDirectory, configDirectory,
        buildDirectory, environmentDirectory);
}

tempo_utils::Result<zuri_tooling::Project>
zuri_tooling::Project::open(const std::filesystem::path &projectDirectoryOrConfigFile)
{
    std::filesystem::path projectConfigFile;
    std::filesystem::path projectDirectory;

    if (std::filesystem::is_regular_file(projectDirectoryOrConfigFile)) {
        projectConfigFile = projectDirectoryOrConfigFile;
        projectDirectory = projectConfigFile.parent_path();
    } else if (std::filesystem::is_directory(projectDirectoryOrConfigFile)) {
        projectDirectory = projectDirectoryOrConfigFile;
        projectConfigFile = projectDirectory / kProjectConfigName;
    } else {
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "zuri project not found at {}", projectDirectoryOrConfigFile.string());
    }

    if (!std::filesystem::exists(projectConfigFile))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing zuri project config file {}", projectConfigFile.string());

    auto srcDirectory = projectDirectory / "src";
    if (!std::filesystem::exists(srcDirectory))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing project src directory {}", srcDirectory.string());

    auto configDirectory = projectDirectory / "config";
    if (!std::filesystem::exists(configDirectory))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing project config directory {}", configDirectory.string());

    auto buildDirectory = projectDirectory / "build";
    if (!std::filesystem::exists(buildDirectory))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing project build directory {}", buildDirectory.string());

    auto environmentDirectory = projectDirectory / kProjectEnvironmentDirectoryName;
    if (!std::filesystem::exists(environmentDirectory))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing project environment directory {}", environmentDirectory.string());

    return Project(projectConfigFile, projectDirectory, srcDirectory, configDirectory,
        buildDirectory, environmentDirectory);
}

inline std::filesystem::path resolve_path(
    const std::filesystem::path &p,
    const std::filesystem::path &base)
{
    if (p.is_absolute())
        return p;
    auto full_path = base / p;
    return full_path.lexically_normal();
}

tempo_utils::Result<zuri_tooling::Project>
zuri_tooling::Project::find(const std::filesystem::path &searchStart)
{
    // the initial search path must exist and be a directory
    if (!std::filesystem::exists(searchStart))
        return Project{};

    // if searchStart is not a directory, then cd to the parent directory
    auto currentDirectory = searchStart;
    if (!std::filesystem::is_directory(currentDirectory)) {
        currentDirectory = currentDirectory.parent_path();
    }

    // check each parent directory for a file called "project.config". if the file is found then we have
    // determined the project root. otherwise if no file is found then project detection failed.
    while (currentDirectory != currentDirectory.root_path()) {
        auto file = currentDirectory / kProjectConfigName;
        if (std::filesystem::exists(file))
            return open(currentDirectory);
        currentDirectory = currentDirectory.parent_path();
    }

    // we were unable to find the project
    return Project{};
}
