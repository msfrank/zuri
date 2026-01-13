
#include <absl/strings/str_split.h>

#include <tempo_config/config_utils.h>
#include <tempo_utils/tempdir_maker.h>
#include <zuri_distributor/distributor_result.h>
#include <zuri_distributor/package_store.h>
#include <zuri_packager/package_extractor.h>

zuri_distributor::PackageStore::PackageStore(const std::filesystem::path &packagesDirectory)
    : m_packagesDirectory(packagesDirectory)
{
    TU_ASSERT (!m_packagesDirectory.empty());
}

std::filesystem::path
zuri_distributor::PackageStore::getPackagesDirectory() const
{
    return m_packagesDirectory;
}

bool
zuri_distributor::PackageStore::containsPackage(const zuri_packager::PackageSpecifier &specifier) const
{
    if (!specifier.isValid())
        return false;
    auto packagePath = specifier.toDirectoryPath(m_packagesDirectory);
    return std::filesystem::is_directory(packagePath);
}

tempo_utils::Result<Option<tempo_config::ConfigMap>>
zuri_distributor::PackageStore::describePackage(const zuri_packager::PackageSpecifier &specifier) const
{
    Option<std::filesystem::path> pathOption;
    TU_ASSIGN_OR_RETURN (pathOption, resolvePackage(specifier));
    if (pathOption.isEmpty())
        return Option<tempo_config::ConfigMap>{};
    tempo_config::ConfigMap packageConfig;
    TU_ASSIGN_OR_RETURN (packageConfig, tempo_config::read_config_map_file(
        pathOption.getValue() / "package.config"));
    return Option(packageConfig);
}

tempo_utils::Result<Option<std::filesystem::path>>
zuri_distributor::PackageStore::resolvePackage(const zuri_packager::PackageSpecifier &specifier) const
{
    if (!specifier.isValid())
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "invalid package specifier");

    auto packagePath = specifier.toDirectoryPath(m_packagesDirectory);
    if (!std::filesystem::is_directory(packagePath))
        return Option<std::filesystem::path>();

    return Option(packagePath);
}

tempo_utils::Result<std::filesystem::path>
zuri_distributor::PackageStore::installPackage(const std::filesystem::path &packagePath)
{
    std::shared_ptr<zuri_packager::PackageReader> reader;
    TU_ASSIGN_OR_RETURN (reader, zuri_packager::PackageReader::open(packagePath));
    return installPackage(reader);
}

tempo_utils::Result<std::filesystem::path>
zuri_distributor::PackageStore::installPackage(std::shared_ptr<zuri_packager::PackageReader> reader)
{
    zuri_packager::PackageExtractorOptions options;
    options.workingRoot = m_packagesDirectory;
    options.destinationRoot = m_packagesDirectory;
    zuri_packager::PackageExtractor extractor(reader, options);
    TU_RETURN_IF_NOT_OK (extractor.configure());
    return extractor.extractPackage();
}

tempo_utils::Status
zuri_distributor::PackageStore::removePackage(const zuri_packager::PackageSpecifier &specifier)
{
    auto absolutePath = specifier.toDirectoryPath(m_packagesDirectory);
    if (!std::filesystem::is_directory(absolutePath))
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "missing package {}", absolutePath.string());

    std::error_code ec;
    if (!std::filesystem::remove_all(absolutePath, ec))
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "failed to remove package {}: {}",
            absolutePath.string(), ec.message());

    return {};
}

tempo_utils::Result<std::shared_ptr<zuri_distributor::PackageStore>>
zuri_distributor::PackageStore::openOrCreate(const std::filesystem::path &packagesDirectory)
{
    if (std::filesystem::exists(packagesDirectory))
        return open(packagesDirectory);

    std::error_code ec;
    std::filesystem::create_directory(packagesDirectory, ec);
    if (ec)
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "failed to create packages directory {}: {}", packagesDirectory.string(), ec.message());

    return std::shared_ptr<PackageStore>(new PackageStore(packagesDirectory));
}

tempo_utils::Result<std::shared_ptr<zuri_distributor::PackageStore>>
zuri_distributor::PackageStore::open(const std::filesystem::path &packagesDirectory)
{
    if (!std::filesystem::is_directory(packagesDirectory))
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "packages directory {} does not exist", packagesDirectory.string());

    return std::shared_ptr<PackageStore>(new PackageStore(packagesDirectory));
}