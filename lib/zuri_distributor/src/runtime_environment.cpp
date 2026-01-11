
#include <zuri_distributor/distributor_result.h>
#include <zuri_distributor/runtime_environment.h>

zuri_distributor::RuntimeEnvironment::RuntimeEnvironment(
    std::shared_ptr<EnvironmentDatabase> environmentDatabase,
    const std::filesystem::path &binDirectory,
    const std::filesystem::path &libDirectory,
    std::shared_ptr<PackageCache> packageStore,
    const RuntimeEnvironmentOptions &options)
    : m_environmentDatabase(std::move(environmentDatabase)),
      m_binDirectory(binDirectory),
      m_libDirectory(libDirectory),
      m_packageStore(std::move(packageStore)),
      m_options(options)
{
    TU_ASSERT (m_environmentDatabase != nullptr);
    TU_ASSERT (m_packageStore != nullptr);
}

zuri_distributor::RuntimeEnvironment::~RuntimeEnvironment()
{
}

tempo_utils::Result<std::shared_ptr<zuri_distributor::RuntimeEnvironment>>
zuri_distributor::RuntimeEnvironment::openOrCreate(
    const std::filesystem::path &environmentDirectory,
    const RuntimeEnvironmentOptions &options)
{
    if (std::filesystem::exists(environmentDirectory))
        return open(environmentDirectory);

    std::error_code ec;
    std::filesystem::create_directory(environmentDirectory, ec);
    if (ec)
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "failed to create runtime environment directory {}", environmentDirectory.string());

    auto binDirectory = environmentDirectory / "bin";
    std::filesystem::create_directory(binDirectory, ec);
    if (ec)
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "failed to create environment bin directory {}", binDirectory.string());

    auto libDirectory = environmentDirectory / "lib";
    std::filesystem::create_directory(libDirectory, ec);
    if (ec)
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "failed to create environment lib directory {}", libDirectory.string());

    auto packagesDirectory = environmentDirectory / "packages";
    std::filesystem::create_directory(packagesDirectory, ec);
    if (ec)
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "failed to create environment packages directory {}", packagesDirectory.string());

    auto environmentDatabaseFile = environmentDirectory / kEnvironmentDatabaseName;
    std::shared_ptr<EnvironmentDatabase> environmentDatabase;
    TU_ASSIGN_OR_RETURN (environmentDatabase, EnvironmentDatabase::openOrCreate(environmentDatabaseFile));

    std::shared_ptr<PackageCache> packageStore;
    TU_ASSIGN_OR_RETURN (packageStore, PackageCache::openOrCreate(packagesDirectory));

    auto runtimeEnvironment = std::shared_ptr<RuntimeEnvironment>(new RuntimeEnvironment(
        environmentDatabase, binDirectory, libDirectory, packageStore, options));
    TU_RETURN_IF_NOT_OK (runtimeEnvironment->configure());

    return runtimeEnvironment;
}

tempo_utils::Result<std::shared_ptr<zuri_distributor::RuntimeEnvironment>>
zuri_distributor::RuntimeEnvironment::open(
    const std::filesystem::path &environmentDirectoryOrDatabaseFile,
    const RuntimeEnvironmentOptions &options)
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
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "runtime environment not found at {}", environmentDirectoryOrDatabaseFile.string());
    }

    auto binDirectory = environmentDirectory / "bin";
    if (!std::filesystem::exists(binDirectory))
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "missing environment bin directory {}", binDirectory.string());

    auto libDirectory = environmentDirectory / "lib";
    if (!std::filesystem::exists(libDirectory))
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "missing environment lib directory {}", libDirectory.string());

    auto packagesDirectory = environmentDirectory / "packages";
    if (!std::filesystem::exists(packagesDirectory))
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "missing environment packages directory {}", packagesDirectory.string());

    if (!std::filesystem::exists(environmentDatabaseFile))
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "missing runtime environment database file {}", environmentDatabaseFile.string());

    std::shared_ptr<EnvironmentDatabase> environmentDatabase;
    TU_ASSIGN_OR_RETURN (environmentDatabase, EnvironmentDatabase::open(environmentDatabaseFile));

    std::shared_ptr<PackageCache> packageStore;
    TU_ASSIGN_OR_RETURN (packageStore, PackageCache::open(packagesDirectory));

    auto runtimeEnvironment = std::shared_ptr<RuntimeEnvironment>(new RuntimeEnvironment(
        environmentDatabase, binDirectory, libDirectory, packageStore, options));
    TU_RETURN_IF_NOT_OK (runtimeEnvironment->configure());

    return runtimeEnvironment;
}

tempo_utils::Status
zuri_distributor::RuntimeEnvironment::configure()
{
    m_loader = std::make_shared<PackageCacheLoader>(m_packageStore);

    return {};
}

std::filesystem::path
zuri_distributor::RuntimeEnvironment::getEnvironmentDatabaseFile() const
{
    return m_environmentDatabase->getDatabaseFilePath();
}

std::filesystem::path
zuri_distributor::RuntimeEnvironment::getBinDirectory() const
{
    return m_binDirectory;
}

std::filesystem::path
zuri_distributor::RuntimeEnvironment::getLibDirectory() const
{
    return m_libDirectory;
}

std::filesystem::path
zuri_distributor::RuntimeEnvironment::getPackagesDirectory() const
{
    return m_packageStore->getCacheDirectory();
}

std::shared_ptr<lyric_runtime::AbstractLoader>
zuri_distributor::RuntimeEnvironment::getLoader() const
{
    return m_loader;
}

bool
zuri_distributor::RuntimeEnvironment::containsPackage(const zuri_packager::PackageSpecifier &specifier) const
{
    return m_packageStore->containsPackage(specifier);
}

tempo_utils::Result<Option<tempo_config::ConfigMap>>
zuri_distributor::RuntimeEnvironment::describePackage(const zuri_packager::PackageSpecifier &specifier) const
{
    return m_packageStore->describePackage(specifier);
}

tempo_utils::Result<Option<std::filesystem::path>>
zuri_distributor::RuntimeEnvironment::resolvePackage(const zuri_packager::PackageSpecifier &specifier) const
{
    return m_packageStore->resolvePackage(specifier);
}

tempo_utils::Result<std::filesystem::path>
zuri_distributor::RuntimeEnvironment::installPackage(const std::filesystem::path &packagePath)
{
    return m_packageStore->installPackage(packagePath);
}

tempo_utils::Result<std::filesystem::path>
zuri_distributor::RuntimeEnvironment::installPackage(std::shared_ptr<zuri_packager::PackageReader> reader)
{
    return m_packageStore->installPackage(reader);
}

tempo_utils::Status
zuri_distributor::RuntimeEnvironment::removePackage(const zuri_packager::PackageSpecifier &specifier)
{
    return m_packageStore->removePackage(specifier);
}