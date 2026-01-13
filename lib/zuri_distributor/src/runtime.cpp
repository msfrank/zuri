
#include <zuri_distributor/distributor_result.h>
#include <zuri_distributor/runtime.h>

zuri_distributor::Runtime::Runtime(
    std::shared_ptr<PackageDatabase> packageDatabase,
    const std::filesystem::path &binDirectory,
    const std::filesystem::path &libDirectory,
    std::shared_ptr<PackageStore> packageStore)
    : m_packageDatabase(std::move(packageDatabase)),
      m_binDirectory(binDirectory),
      m_libDirectory(libDirectory),
      m_packageStore(std::move(packageStore))
{
    TU_ASSERT (m_packageDatabase != nullptr);
    TU_ASSERT (m_packageStore != nullptr);
}

zuri_distributor::Runtime::~Runtime()
{
}

inline bool
symlink_libs(
    const std::filesystem::path &sourceLibDir,
    const std::filesystem::path &destLibDir,
    std::error_code &ec)
{
    std::filesystem::directory_iterator sourceIterator(std::filesystem::absolute(sourceLibDir), ec);
    if (ec)
        return false;
    for (const auto &entry : sourceIterator) {
        auto from = std::filesystem::absolute(entry.path());
        auto to = destLibDir / entry.path().filename();
        if (entry.is_directory()) {
            std::filesystem::create_directory_symlink(from, to, ec);
        } else if (entry.is_regular_file()) {
            std::filesystem::create_symlink(from, to, ec);
        }
        if (ec)
            return false;
    }
    return true;
}

tempo_utils::Result<std::shared_ptr<zuri_distributor::Runtime>>
zuri_distributor::Runtime::openOrCreate(
    const std::filesystem::path &runtimeRoot,
    const RuntimeOpenOrCreateOptions &options)
{
    auto packageDatabaseFile = runtimeRoot / kPackagesDatabaseName;
    if (std::filesystem::exists(packageDatabaseFile)) {
        if (options.exclusive)
            return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
                "zuri runtime {} already exists", runtimeRoot.string());
        return open(runtimeRoot);
    }

    // create the packages database
    std::shared_ptr<PackageDatabase> packageDatabase;
    TU_ASSIGN_OR_RETURN (packageDatabase, PackageDatabase::openOrCreate(packageDatabaseFile));

    std::error_code ec;

    // create the bin directory
    auto binDirectory = runtimeRoot / "bin";
    std::filesystem::create_directory(binDirectory, ec);
    if (ec)
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "failed to create runtime bin directory {}", binDirectory.string());

    // create lib directory
    auto libDirectory = runtimeRoot / "lib";
    std::filesystem::create_directory(libDirectory, ec);
    if (ec)
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "failed to create runtime lib directory {}", libDirectory.string());

    // create symlinks to all libraries in the distribution lib directory
    if (!options.distributionLibDir.empty()) {
        if (!symlink_libs(options.distributionLibDir, libDirectory, ec))
            return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
                "failed to link distribution libraries from {}; {}",
                options.distributionLibDir.string(), ec.message());
    }

    // create symlinks to all libraries in the lib directory of each component
    for (const auto &extraLibDir : options.extraLibDirs) {
        if (!symlink_libs(extraLibDir, libDirectory, ec))
            return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
                "failed to link extra libraries from {}; {}",
                extraLibDir.string(), ec.message());
    }

    //
    auto packagesDirectory = runtimeRoot / "packages";
    std::filesystem::create_directory(packagesDirectory, ec);
    if (ec)
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "failed to create runtime packages directory {}", packagesDirectory.string());

    std::shared_ptr<PackageStore> packageStore;
    TU_ASSIGN_OR_RETURN (packageStore, PackageStore::openOrCreate(packagesDirectory));

    auto runtime = std::shared_ptr<Runtime>(
        new Runtime(packageDatabase, binDirectory, libDirectory, packageStore));
    TU_RETURN_IF_NOT_OK (runtime->configure());

    return runtime;
}

tempo_utils::Result<std::shared_ptr<zuri_distributor::Runtime>>
zuri_distributor::Runtime::open(const std::filesystem::path &runtimeRoot)
{
    if (!std::filesystem::exists(runtimeRoot))
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "runtime not found at {}", runtimeRoot.string());

    auto binDirectory = runtimeRoot / "bin";
    if (!std::filesystem::exists(binDirectory))
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "missing runtime bin directory {}", binDirectory.string());

    auto libDirectory = runtimeRoot / "lib";
    if (!std::filesystem::exists(libDirectory))
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "missing runtime lib directory {}", libDirectory.string());

    auto packagesDirectory = runtimeRoot / "packages";
    if (!std::filesystem::exists(packagesDirectory))
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "missing runtime packages directory {}", packagesDirectory.string());

    auto packageDatabaseFile = runtimeRoot / kPackagesDatabaseName;
    if (!std::filesystem::exists(packageDatabaseFile))
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "missing runtime packages database file {}", packageDatabaseFile.string());

    std::shared_ptr<PackageDatabase> packageDatabase;
    TU_ASSIGN_OR_RETURN (packageDatabase, PackageDatabase::open(packageDatabaseFile));

    std::shared_ptr<PackageStore> packageStore;
    TU_ASSIGN_OR_RETURN (packageStore, PackageStore::open(packagesDirectory));

    auto runtime = std::shared_ptr<Runtime>(
        new Runtime(packageDatabase, binDirectory, libDirectory, packageStore));
    TU_RETURN_IF_NOT_OK (runtime->configure());

    return runtime;
}

tempo_utils::Status
zuri_distributor::Runtime::configure()
{
    m_loader = std::make_shared<PackageCacheLoader>(m_packageStore);

    return {};
}

std::filesystem::path
zuri_distributor::Runtime::getPackagesDatabaseFile() const
{
    return m_packageDatabase->getDatabaseFilePath();
}

std::filesystem::path
zuri_distributor::Runtime::getBinDirectory() const
{
    return m_binDirectory;
}

std::filesystem::path
zuri_distributor::Runtime::getLibDirectory() const
{
    return m_libDirectory;
}

std::filesystem::path
zuri_distributor::Runtime::getPackagesDirectory() const
{
    return m_packageStore->getPackagesDirectory();
}

std::shared_ptr<lyric_runtime::AbstractLoader>
zuri_distributor::Runtime::getLoader() const
{
    return m_loader;
}

bool
zuri_distributor::Runtime::containsPackage(const zuri_packager::PackageSpecifier &specifier) const
{
    return m_packageStore->containsPackage(specifier);
}

tempo_utils::Result<Option<tempo_config::ConfigMap>>
zuri_distributor::Runtime::describePackage(const zuri_packager::PackageSpecifier &specifier) const
{
    return m_packageStore->describePackage(specifier);
}

tempo_utils::Result<Option<std::filesystem::path>>
zuri_distributor::Runtime::resolvePackage(const zuri_packager::PackageSpecifier &specifier) const
{
    return m_packageStore->resolvePackage(specifier);
}

tempo_utils::Result<std::filesystem::path>
zuri_distributor::Runtime::installPackage(const std::filesystem::path &packagePath)
{
    return m_packageStore->installPackage(packagePath);
}

tempo_utils::Result<std::filesystem::path>
zuri_distributor::Runtime::installPackage(std::shared_ptr<zuri_packager::PackageReader> reader)
{
    return m_packageStore->installPackage(reader);
}

tempo_utils::Status
zuri_distributor::Runtime::removePackage(const zuri_packager::PackageSpecifier &specifier)
{
    return m_packageStore->removePackage(specifier);
}