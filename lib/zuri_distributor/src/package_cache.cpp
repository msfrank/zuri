
#include <absl/strings/str_split.h>

#include <tempo_config/config_utils.h>
#include <tempo_utils/tempdir_maker.h>
#include <zuri_distributor/distributor_result.h>
#include <zuri_distributor/package_cache.h>
#include <zuri_packager/package_extractor.h>

zuri_distributor::PackageCache::PackageCache(const std::filesystem::path &cacheDirectory)
    : m_cacheDirectory(cacheDirectory)
{
    TU_ASSERT (!m_cacheDirectory.empty());
}

std::filesystem::path
zuri_distributor::PackageCache::getCacheDirectory() const
{
    return m_cacheDirectory;
}

bool
zuri_distributor::PackageCache::containsPackage(const zuri_packager::PackageSpecifier &specifier) const
{
    if (!specifier.isValid())
        return false;
    auto packagePath = specifier.toDirectoryPath(m_cacheDirectory);
    return std::filesystem::is_directory(packagePath);
}

tempo_utils::Result<Option<tempo_config::ConfigMap>>
zuri_distributor::PackageCache::describePackage(const zuri_packager::PackageSpecifier &specifier) const
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
zuri_distributor::PackageCache::resolvePackage(const zuri_packager::PackageSpecifier &specifier) const
{
    if (!specifier.isValid())
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "invalid package specifier");

    auto packagePath = specifier.toDirectoryPath(m_cacheDirectory);
    if (!std::filesystem::is_directory(packagePath))
        return Option<std::filesystem::path>();

    return Option(packagePath);
}

tempo_utils::Result<std::filesystem::path>
zuri_distributor::PackageCache::installPackage(const std::filesystem::path &packagePath)
{
    std::shared_ptr<zuri_packager::PackageReader> reader;
    TU_ASSIGN_OR_RETURN (reader, zuri_packager::PackageReader::open(packagePath));
    return installPackage(reader);
}

tempo_utils::Result<std::filesystem::path>
zuri_distributor::PackageCache::installPackage(std::shared_ptr<zuri_packager::PackageReader> reader)
{
    zuri_packager::PackageExtractorOptions options;
    options.workingRoot = m_cacheDirectory;
    options.destinationRoot = m_cacheDirectory;
    zuri_packager::PackageExtractor extractor(reader, options);
    TU_RETURN_IF_NOT_OK (extractor.configure());
    return extractor.extractPackage();
}

tempo_utils::Status
zuri_distributor::PackageCache::removePackage(const zuri_packager::PackageSpecifier &specifier)
{
    auto absolutePath = specifier.toDirectoryPath(m_cacheDirectory);
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

tempo_utils::Result<std::shared_ptr<zuri_distributor::PackageCache>>
zuri_distributor::PackageCache::openOrCreate(const std::filesystem::path &cacheDirectory)
{
    auto cacheRoot = cacheDirectory.parent_path();
    auto cacheName = cacheDirectory.filename();
    return openOrCreate(cacheRoot, cacheName.string());
}

tempo_utils::Result<std::shared_ptr<zuri_distributor::PackageCache>>
zuri_distributor::PackageCache::openOrCreate(
    const std::filesystem::path &cacheRoot,
    std::string_view cacheName)
{
    auto packageCachePath = cacheRoot / cacheName;
    if (std::filesystem::exists(packageCachePath))
        return open(packageCachePath);

    if (!std::filesystem::is_directory(cacheRoot))
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "package cache root {} does not exist", cacheRoot.string());

    tempo_utils::TempdirMaker cacheTempDir(cacheRoot, absl::StrCat(cacheName, ".XXXXXXXX"));
    TU_RETURN_IF_NOT_OK (cacheTempDir.getStatus());

    std::error_code ec;
    std::filesystem::rename(cacheTempDir.getTempdir(), packageCachePath, ec);
    if (ec)
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "failed to rename {}: {}", packageCachePath.string(), ec.message());

    return std::shared_ptr<PackageCache>(new PackageCache(packageCachePath));
}

tempo_utils::Result<std::shared_ptr<zuri_distributor::PackageCache>>
zuri_distributor::PackageCache::open(const std::filesystem::path &cacheDirectory)
{
    if (!std::filesystem::is_directory(cacheDirectory))
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "package cache {} does not exist", cacheDirectory.string());

    return std::shared_ptr<PackageCache>(new PackageCache(cacheDirectory));
}