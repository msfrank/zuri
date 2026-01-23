
#include <tempo_config/base_conversions.h>
#include <tempo_config/config_builder.h>
#include <tempo_config/config_utils.h>
#include <tempo_config/parse_config.h>
#include <zuri_tooling/project.h>
#include <zuri_tooling/tooling_result.h>

#include "zuri_tooling/environment.h"

zuri_tooling::Project::Project()
{
}

zuri_tooling::Project::Project(
    const std::filesystem::path &projectConfigFile,
    const std::filesystem::path &projectDirectory,
    const std::filesystem::path &configDirectory,
    const std::filesystem::path &targetsDirectory,
    const std::filesystem::path &buildDirectory,
    const std::filesystem::path &buildEnvironmentDirectory,
    bool linked)
    : m_priv(std::make_shared<Priv>(
        projectConfigFile,
        projectDirectory,
        configDirectory,
        targetsDirectory,
        buildDirectory,
        buildEnvironmentDirectory,
        linked))
{
    TU_ASSERT (!m_priv->projectConfigFile.empty());
    TU_ASSERT (!m_priv->projectDirectory.empty());
    TU_ASSERT (!m_priv->targetsDirectory.empty());
    TU_ASSERT (!m_priv->buildDirectory.empty());
    TU_ASSERT (!m_priv->buildEnvironmentDirectory.empty());
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

bool
zuri_tooling::Project::isLinked() const
{
    if (m_priv == nullptr)
        return false;
    return m_priv->linked;
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
zuri_tooling::Project::getTargetsDirectory() const
{
    if (m_priv)
        return m_priv->targetsDirectory;
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
zuri_tooling::Project::getBuildEnvironmentDirectory() const
{
    if (m_priv)
        return m_priv->buildEnvironmentDirectory;
    return {};
}

tempo_utils::Result<zuri_tooling::Project>
zuri_tooling::Project::create(
    const std::filesystem::path &projectDirectory,
    const ProjectOpenOrCreateOptions &options)
{
    TU_ASSERT (options.linkedProject.empty());

    std::error_code ec;

    // create the project root
    std::filesystem::create_directory(projectDirectory, ec);
    if (ec)
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "failed to create zuri project directory {}; {}",
            projectDirectory.string(), ec.message());

    // create the targets directory
    auto targetsDirectory = projectDirectory / "targets";
    std::filesystem::create_directory(targetsDirectory, ec);
    if (ec)
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "failed to create project targets directory {}; {}",
            targetsDirectory.string(), ec.message());

    // create the config directory
    auto configDirectory = projectDirectory / "config";
    std::filesystem::create_directory(configDirectory, ec);
    if (ec)
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "failed to create project config directory {}; {}",
            configDirectory.string(), ec.message());

    // create the .zuribuild directory
    auto buildDirectory = projectDirectory / kProjectBuildDirectoryName;
    std::filesystem::create_directory(buildDirectory, ec);
    if (ec)
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "failed to create project build directory {}; {}",
            buildDirectory.string(), ec.message());

    // create the .zuribuild/env directory
    EnvironmentOpenOrCreateOptions environmentOpenOrCreateOptions;
    environmentOpenOrCreateOptions.distribution = options.distribution;
    environmentOpenOrCreateOptions.extraLibDirs = options.extraLibDirs;
    environmentOpenOrCreateOptions.exclusive = true;
    auto buildEnvironmentDirectory = buildDirectory / "env";
    Environment environment;
    TU_ASSIGN_OR_RETURN (environment, Environment::openOrCreate(
        buildEnvironmentDirectory, environmentOpenOrCreateOptions));

    auto projectConfigFile = projectDirectory / kProjectConfigName;

    // write the project.config
    TU_RETURN_IF_NOT_OK (tempo_config::write_config_file(options.projectMap, projectConfigFile));

    return Project(projectConfigFile, projectDirectory, configDirectory, targetsDirectory,
        buildDirectory, buildEnvironmentDirectory, /* linked= */ false);

}

static tempo_utils::Status
canonicalize(std::filesystem::path &path)
{
    std::error_code ec;
    auto canonicalPath = std::filesystem::canonical(path, ec);
    if (ec)
        return zuri_tooling::ToolingStatus::forCondition(
            zuri_tooling::ToolingCondition::kToolingInvariant,
            "failed to canonicalize path {}; {}", path.string(), ec.message());
    path = std::move(canonicalPath);
    return {};
}

tempo_utils::Result<zuri_tooling::Project>
zuri_tooling::Project::link(
    const std::filesystem::path &projectDirectory,
    const ProjectOpenOrCreateOptions &options)
{
    TU_ASSERT (!options.linkedProject.empty());
    auto linkedProjectDirectory = options.linkedProject;

    if (!std::filesystem::is_directory(linkedProjectDirectory))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "linked project directory {} does not exist",
            linkedProjectDirectory.string());
    TU_RETURN_IF_NOT_OK (canonicalize(linkedProjectDirectory));

    std::error_code ec;

    // create the project root
    std::filesystem::create_directory(projectDirectory, ec);
    if (ec)
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "failed to create zuri project directory {}; {}",
            projectDirectory.string(), ec.message());

    // create the .zuribuild directory
    auto buildDirectory = projectDirectory / kProjectBuildDirectoryName;
    std::filesystem::create_directory(buildDirectory, ec);
    if (ec)
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "failed to create project build directory {}; {}",
            buildDirectory.string(), ec.message());

    // create the .zuribuild/env directory
    EnvironmentOpenOrCreateOptions environmentOpenOrCreateOptions;
    environmentOpenOrCreateOptions.distribution = options.distribution;
    environmentOpenOrCreateOptions.extraLibDirs = options.extraLibDirs;
    environmentOpenOrCreateOptions.exclusive = true;
    auto buildEnvironmentDirectory = buildDirectory / "env";
    Environment environment;
    TU_ASSIGN_OR_RETURN (environment, Environment::openOrCreate(
        buildEnvironmentDirectory, environmentOpenOrCreateOptions));

    // determine the required linked targets directory
    std::filesystem::path linkedTargetsDirectory;
    if (!options.linkedTargetsDirectory.empty()) {
        linkedTargetsDirectory = options.linkedTargetsDirectory;
    } else {
        linkedTargetsDirectory = linkedProjectDirectory / "targets";
    }
    if (!std::filesystem::is_directory(linkedTargetsDirectory))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing linked targets directory {}", linkedTargetsDirectory.string());
    TU_RETURN_IF_NOT_OK (canonicalize(linkedTargetsDirectory));

    // determine the optional linked config directory
    std::filesystem::path linkedConfigDirectory;
    if (!options.linkedConfigDirectory.empty()) {
        linkedConfigDirectory = options.linkedConfigDirectory;
        if (!std::filesystem::is_directory(linkedConfigDirectory))
            return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
                "missing linked config directory {}", linkedConfigDirectory.string());
    } else {
        linkedConfigDirectory = linkedProjectDirectory / "config";
        if (!std::filesystem::exists(linkedConfigDirectory)) {
            linkedConfigDirectory.clear();
        }
    }
    if (!linkedConfigDirectory.empty()) {
        TU_RETURN_IF_NOT_OK (canonicalize(linkedConfigDirectory));
    }

    // determine the linked project config file
    std::filesystem::path linkedProjectConfigFile;
    if (!options.linkedProjectConfigFile.empty()) {
        linkedProjectConfigFile = options.linkedProjectConfigFile;
    } else {
        linkedProjectConfigFile = linkedProjectDirectory / kProjectConfigName;
    }
    if (!std::filesystem::is_regular_file(linkedProjectConfigFile))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing linked project config file {}", linkedProjectConfigFile.string());
    TU_RETURN_IF_NOT_OK (canonicalize(linkedProjectConfigFile));

    // write the .zuribuild/source.config file
    auto sourceMapBuilder = tempo_config::startMap()
        .put("linkedProject", tempo_config::valueNode(linkedProjectDirectory.string()))
        .put("targetsDirectory", tempo_config::valueNode(linkedTargetsDirectory.string()));
    if (!linkedConfigDirectory.empty()) {
        sourceMapBuilder = sourceMapBuilder
            .put("configDirectory", tempo_config::valueNode(linkedConfigDirectory.string()));
    }
    auto sourceMap = sourceMapBuilder.buildMap();
    auto sourceConfigFile = buildDirectory / kSourceConfigName;
    TU_RETURN_IF_NOT_OK (tempo_config::write_config_file(sourceMap, sourceConfigFile));

    // create symlink to the project.config file
    auto projectConfigFile = projectDirectory / kProjectConfigName;
    std::filesystem::create_symlink(linkedProjectConfigFile, projectConfigFile, ec);
    if (ec)
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "failed to link project config file {}; {}",
            linkedProjectConfigFile.string(), ec.message());

    return Project(projectConfigFile, projectDirectory, linkedConfigDirectory, linkedTargetsDirectory,
        buildDirectory, buildEnvironmentDirectory, /* linked= */ true);
}

tempo_utils::Result<zuri_tooling::Project>
zuri_tooling::Project::openOrCreate(
    const std::filesystem::path &projectDirectory,
    const ProjectOpenOrCreateOptions &options)
{
    if (std::filesystem::exists(projectDirectory)) {
        if (options.exclusive)
            return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
                "zuri project directory {} already exists", projectDirectory.string());
        return open(projectDirectory);
    }

    tempo_utils::Result<Project> result;
    if (!options.linkedProject.empty()) {
        result = link(projectDirectory, options);
    } else {
        result = create(projectDirectory, options);
    }
    if (result.isStatus()) {
        std::filesystem::remove_all(projectDirectory);
    }
    return result;
}

tempo_utils::Result<zuri_tooling::Project>
zuri_tooling::Project::open(const std::filesystem::path &projectDirectoryOrConfigFile)
{
    std::filesystem::path projectConfigFile;
    std::filesystem::path projectDirectory;

    // determine the project config file and project directory
    if (std::filesystem::is_regular_file(projectDirectoryOrConfigFile)) {
        projectConfigFile = projectDirectoryOrConfigFile;
        projectDirectory = projectDirectoryOrConfigFile.parent_path();
    } else if (std::filesystem::is_directory(projectDirectoryOrConfigFile)) {
        projectDirectory = projectDirectoryOrConfigFile;
        projectConfigFile = projectDirectoryOrConfigFile / kProjectConfigName;
    } else {
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "zuri project not found at {}", projectDirectoryOrConfigFile.string());
    }

    auto buildDirectory = projectDirectory / kProjectBuildDirectoryName;
    auto buildEnvironmentDirectory = buildDirectory / "env";
    auto targetsDirectory = projectDirectory / "targets";
    auto configDirectory = projectDirectory / "config";
    bool linked = false;

    // if project is linked then load paths from source.config

    auto sourceConfigFile = projectDirectory / kProjectBuildDirectoryName / kSourceConfigName;
    if (std::filesystem::exists(sourceConfigFile)) {
        tempo_config::ConfigMap sourceMap;
        TU_ASSIGN_OR_RETURN (sourceMap, tempo_config::read_config_map_file(sourceConfigFile));
        tempo_config::PathParser linkedProjectParser;
        TU_RETURN_IF_NOT_OK (tempo_config::parse_config(projectDirectory, linkedProjectParser,
            sourceMap, "linkedProject"));
        tempo_config::PathParser targetsDirectoryParser;
        TU_RETURN_IF_NOT_OK (tempo_config::parse_config(targetsDirectory, targetsDirectoryParser,
            sourceMap, "targetsDirectory"));
        tempo_config::PathParser configDirectoryParser(std::filesystem::path{});
        TU_RETURN_IF_NOT_OK (tempo_config::parse_config(configDirectory, configDirectoryParser,
            sourceMap, "configDirectory"));
        linked = true;
    }

    // verify project config file exists
    if (!std::filesystem::exists(projectConfigFile))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing zuri project config file {}", projectConfigFile.string());

    // verify project directory exists
    if (!std::filesystem::exists(projectDirectory))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing project directory {}", projectDirectory.string());

    // verify build directory exists
    if (!std::filesystem::exists(buildDirectory))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing project build directory {}", buildDirectory.string());

    // verify build environment directory exists
    if (!std::filesystem::exists(buildEnvironmentDirectory))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing project build environment directory {}", buildEnvironmentDirectory.string());

    // verify targets directory exists
    if (!std::filesystem::exists(targetsDirectory))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing project targets directory {}", targetsDirectory.string());

    // if optional config directory is specified then verify it exists
    if (!configDirectory.empty() && !std::filesystem::exists(configDirectory))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing project config directory {}", configDirectory.string());

    return Project(projectConfigFile, projectDirectory, configDirectory, targetsDirectory,
        buildDirectory, buildEnvironmentDirectory, linked);
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
