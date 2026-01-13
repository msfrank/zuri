
#include <tempo_config/config_utils.h>
#include <zuri_distributor/runtime.h>
#include <zuri_tooling/environment.h>
#include <zuri_tooling/tooling_result.h>

zuri_tooling::Environment::Environment()
{
}

zuri_tooling::Environment::Environment(
    const std::filesystem::path &environmentConfigFile,
    const std::filesystem::path &environmentDirectory,
    const std::filesystem::path &binDirectory,
    const std::filesystem::path &libDirectory,
    const std::filesystem::path &packagesDirectory,
    const std::filesystem::path &packagesDatabaseFile,
    const std::filesystem::path &configDirectory)
    : m_priv(std::make_shared<Priv>(
        environmentConfigFile,
        environmentDirectory,
        binDirectory,
        libDirectory,
        packagesDirectory,
        packagesDatabaseFile,
        configDirectory))
{
    TU_ASSERT (!m_priv->environmentConfigFile.empty());
    TU_ASSERT (!m_priv->environmentDirectory.empty());
    TU_ASSERT (!m_priv->binDirectory.empty());
    TU_ASSERT (!m_priv->libDirectory.empty());
    TU_ASSERT (!m_priv->packagesDirectory.empty());
    TU_ASSERT (!m_priv->packagesDatabaseFile.empty());
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
zuri_tooling::Environment::getEnvironmentConfigFile() const
{
    if (m_priv)
        return m_priv->environmentConfigFile;
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
zuri_tooling::Environment::getPackagesDatabaseFile() const
{
    if (m_priv)
        return m_priv->packagesDatabaseFile;
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
zuri_tooling::Environment::openOrCreate(
    const std::filesystem::path &environmentDirectory,
    const EnvironmentOpenOrCreateOptions &options)
{
    if (std::filesystem::exists(environmentDirectory)) {
        if (options.exclusive)
            return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
                "zuri environment directory {} already exists", environmentDirectory.string());
        return open(environmentDirectory);
    }

    std::error_code ec;

    // create the environment root
    std::filesystem::create_directory(environmentDirectory, ec);
    if (ec)
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "failed to create zuri environment directory {}", environmentDirectory.string());

    // create the runtime
    std::shared_ptr<zuri_distributor::Runtime> runtime;
    zuri_distributor::RuntimeOpenOrCreateOptions runtimeOpenOrCreateOptions;
    if (options.distribution.isValid()) {
        runtimeOpenOrCreateOptions.distributionLibDir = options.distribution.getLibDirectory();
    }
    runtimeOpenOrCreateOptions.extraLibDirs = options.extraLibDirs;
    runtimeOpenOrCreateOptions.exclusive = true;
    TU_ASSIGN_OR_RETURN (runtime, zuri_distributor::Runtime::openOrCreate(
        environmentDirectory, runtimeOpenOrCreateOptions));

    // create the config directory
    auto configDirectory = environmentDirectory / "config";
    std::filesystem::create_directory(configDirectory, ec);
    if (ec)
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "failed to create environment config directory {}", configDirectory.string());

    // write the environment.config
    auto environmentConfigFile = environmentDirectory / kEnvironmentConfigName;
    TU_RETURN_IF_NOT_OK (tempo_config::write_config_file(options.projectMap, environmentConfigFile));

    auto packagesDatabaseFile = runtime->getPackagesDatabaseFile();
    auto binDirectory = runtime->getBinDirectory();
    auto libDirectory = runtime->getLibDirectory();
    auto packagesDirectory = runtime->getPackagesDirectory();

    return Environment(environmentConfigFile, environmentDirectory, binDirectory, libDirectory,
        packagesDirectory, packagesDatabaseFile, configDirectory);
}

tempo_utils::Result<zuri_tooling::Environment>
zuri_tooling::Environment::open(const std::filesystem::path &environmentDirectoryOrConfigFile)
{
    std::filesystem::path environmentConfigFile;
    std::filesystem::path environmentDirectory;

    if (std::filesystem::is_regular_file(environmentDirectoryOrConfigFile)) {
        environmentConfigFile = environmentDirectoryOrConfigFile;
        environmentDirectory = environmentConfigFile.parent_path();
    } else if (std::filesystem::is_directory(environmentDirectoryOrConfigFile)) {
        environmentDirectory = environmentDirectoryOrConfigFile;
        environmentConfigFile = environmentDirectory / kEnvironmentConfigName;
    } else {
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "zuri environment not found at {}", environmentDirectoryOrConfigFile.string());
    }

    if (!std::filesystem::exists(environmentConfigFile))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing zuri environment config file {}", environmentConfigFile.string());

    std::shared_ptr<zuri_distributor::Runtime> runtime;
    TU_ASSIGN_OR_RETURN (runtime, zuri_distributor::Runtime::open(environmentDirectory));

    auto packagesDatabaseFile = runtime->getPackagesDatabaseFile();
    auto binDirectory = runtime->getBinDirectory();
    auto libDirectory = runtime->getLibDirectory();
    auto packagesDirectory = runtime->getPackagesDirectory();

    auto configDirectory = environmentDirectory / "config";
    if (!std::filesystem::exists(configDirectory))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing environment config directory {}", configDirectory.string());

    return Environment(environmentConfigFile, environmentDirectory, binDirectory, libDirectory,
        packagesDirectory, packagesDatabaseFile, configDirectory);
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

    // check each parent directory for a file called "environment.config". if the file is found then we have
    // determined the environment root. otherwise if no file is found then environment detection failed.
    while (currentDirectory != currentDirectory.root_path()) {
        auto file = currentDirectory / kEnvironmentConfigName;
        if (std::filesystem::exists(file))
            return open(currentDirectory);
        currentDirectory = currentDirectory.parent_path();
    }

    // we were unable to find the environment
    return Environment{};
}