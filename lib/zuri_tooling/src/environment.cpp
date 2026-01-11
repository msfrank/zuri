
#include <zuri_tooling/environment.h>
#include <zuri_tooling/tooling_result.h>

zuri_tooling::Environment::Environment()
{
}

zuri_tooling::Environment::Environment(
    const std::filesystem::path &environmentDatabaseFile,
    const std::filesystem::path &environmentDirectory,
    const std::filesystem::path &binDirectory,
    const std::filesystem::path &libDirectory,
    const std::filesystem::path &packagesDirectory,
    const std::filesystem::path &configDirectory)
    : m_priv(std::make_shared<Priv>(
        environmentDatabaseFile,
        environmentDirectory,
        binDirectory,
        libDirectory,
        packagesDirectory,
        configDirectory))
{
    TU_ASSERT (!m_priv->environmentDatabaseFile.empty());
    TU_ASSERT (!m_priv->environmentDirectory.empty());
    TU_ASSERT (!m_priv->binDirectory.empty());
    TU_ASSERT (!m_priv->libDirectory.empty());
    TU_ASSERT (!m_priv->packagesDirectory.empty());
    TU_ASSERT (!m_priv->configDirectory.empty());
}

zuri_tooling::Environment::Environment(const Environment &other)
    : m_priv(other.m_priv)
{
}

bool
zuri_tooling::Environment::isValid() const
{
    return m_priv != nullptr;
}

std::filesystem::path
zuri_tooling::Environment::getEnvironmentDatabaseFile() const
{
    if (m_priv)
        return m_priv->environmentDatabaseFile;
    return {};
}

std::filesystem::path
zuri_tooling::Environment::getEnvironmentDirectory() const
{
    if (m_priv)
        return m_priv->environmentDirectory;
    return {};
}

std::filesystem::path
zuri_tooling::Environment::getBinDirectory() const
{
    if (m_priv)
        return m_priv->binDirectory;
    return {};
}

std::filesystem::path
zuri_tooling::Environment::getLibDirectory() const
{
    if (m_priv)
        return m_priv->libDirectory;
    return {};
}

std::filesystem::path
zuri_tooling::Environment::getPackagesDirectory() const
{
    if (m_priv)
        return m_priv->packagesDirectory;
    return {};
}

std::filesystem::path
zuri_tooling::Environment::getConfigDirectory() const
{
    if (m_priv)
        return m_priv->configDirectory;
    return {};
}

tempo_utils::Result<zuri_tooling::Environment>
zuri_tooling::Environment::openOrCreate(const std::filesystem::path &environmentDirectory)
{
    if (std::filesystem::exists(environmentDirectory))
        return open(environmentDirectory);

    std::error_code ec;
    std::filesystem::create_directory(environmentDirectory, ec);
    if (ec)
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "failed to create zuri environment directory {}", environmentDirectory.string());

    auto binDirectory = environmentDirectory / "bin";
    std::filesystem::create_directory(binDirectory, ec);
    if (ec)
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "failed to create environment bin directory {}", binDirectory.string());

    auto libDirectory = environmentDirectory / "lib";
    std::filesystem::create_directory(libDirectory, ec);
    if (ec)
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "failed to create environment lib directory {}", libDirectory.string());

    auto packagesDirectory = environmentDirectory / "packages";
    std::filesystem::create_directory(packagesDirectory, ec);
    if (ec)
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "failed to create environment packages directory {}", packagesDirectory.string());

    auto configDirectory = environmentDirectory / "config";
    std::filesystem::create_directory(configDirectory, ec);
    if (ec)
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "failed to create environment config directory {}", configDirectory.string());

    return Environment(environmentDirectory, binDirectory, libDirectory, packagesDirectory, configDirectory);
}

tempo_utils::Result<zuri_tooling::Environment>
zuri_tooling::Environment::open(const std::filesystem::path &environmentDirectoryOrDatabaseFile)
{
    std::filesystem::path environmentDatabaseFile;
    std::filesystem::path environmentDirectory;

    if (std::filesystem::is_regular_file(environmentDirectoryOrDatabaseFile)) {
        environmentDatabaseFile = environmentDirectoryOrDatabaseFile;
        environmentDirectory = environmentDatabaseFile.parent_path();
    } else if (std::filesystem::is_directory(environmentDirectoryOrDatabaseFile)) {
        environmentDirectory = environmentDirectoryOrDatabaseFile;
        environmentDatabaseFile = environmentDirectory / kEnvironmentDatabaseName;
    } else {
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "zuri environment not found at {}", environmentDirectoryOrDatabaseFile.string());
    }

    if (!std::filesystem::exists(environmentDatabaseFile))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing zuri environment database file {}", environmentDatabaseFile.string());

    auto binDirectory = environmentDirectory / "bin";
    if (!std::filesystem::exists(binDirectory))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing environment bin directory {}", binDirectory.string());

    auto libDirectory = environmentDirectory / "lib";
    if (!std::filesystem::exists(libDirectory))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing environment lib directory {}", libDirectory.string());

    auto packagesDirectory = environmentDirectory / "packages";
    if (!std::filesystem::exists(packagesDirectory))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing environment packages directory {}", packagesDirectory.string());

    auto configDirectory = environmentDirectory / "config";
    if (!std::filesystem::exists(configDirectory))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing environment config directory {}", configDirectory.string());

    return Environment(environmentDatabaseFile, environmentDirectory, binDirectory, libDirectory,
        packagesDirectory, configDirectory);
}

tempo_utils::Result<zuri_tooling::Environment>
zuri_tooling::Environment::find(const std::filesystem::path &searchStart)
{
    // the initial search path must exist and be a directory
    if (!std::filesystem::exists(searchStart))
        return Environment{};

    // if searchStart is not a directory, then cd to the parent directory
    auto currentDirectory = searchStart;
    if (!std::filesystem::is_directory(currentDirectory)) {
        currentDirectory = currentDirectory.parent_path();
    }

    // check each parent directory for a file called "environment.db". if the file is found then we have
    // determined the environment root. otherwise if no file is found then environment detection failed.
    while (currentDirectory != currentDirectory.root_path()) {
        auto file = currentDirectory / kEnvironmentDatabaseName;
        if (std::filesystem::exists(file))
            return open(currentDirectory);
        currentDirectory = currentDirectory.parent_path();
    }

    // we were unable to find the environment
    return Environment{};
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

tempo_utils::Result<zuri_tooling::Environment>
zuri_tooling::Environment::relativeTo(const std::filesystem::path &programLocation)
{
    if (!std::filesystem::is_symlink(programLocation))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "invalid program location {}", programLocation.string());
    auto binDirectory = programLocation.parent_path();
    auto environmentDirectory = binDirectory.parent_path();

    auto environmentDatabaseFile = environmentDirectory / kEnvironmentDatabaseName;
    if (!std::filesystem::exists(environmentDatabaseFile))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing environment database file {}", environmentDatabaseFile.string());

    auto libDirectory = resolve_path(std::filesystem::path{RELATIVE_LIB_DIR}, binDirectory);
    if (!std::filesystem::exists(libDirectory))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing environment lib directory {}", libDirectory.string());

    auto packagesDirectory = resolve_path(std::filesystem::path{RELATIVE_PACKAGES_DIR}, binDirectory);
    if (!std::filesystem::exists(packagesDirectory))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing environment packages directory {}", packagesDirectory.string());

    auto configDirectory = resolve_path(std::filesystem::path{RELATIVE_CONFIG_DIR}, binDirectory);
    if (!std::filesystem::exists(configDirectory))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing distribution config directory {}", configDirectory.string());

    return Environment(environmentDatabaseFile, environmentDirectory, binDirectory, libDirectory,
        packagesDirectory, configDirectory);
}
