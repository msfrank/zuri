
#include <absl/strings/str_join.h>
#include <absl/strings/str_split.h>

#include <lyric_common/common_types.h>
#include <lyric_common/plugin.h>
#include <lyric_runtime/library_plugin.h>
#include <tempo_utils/file_reader.h>
#include <tempo_utils/library_loader.h>
#include <tempo_utils/log_stream.h>
#include <tempo_utils/platform.h>
#include <zuri_distributor/distributor_result.h>
#include <zuri_distributor/package_cache_loader.h>

zuri_distributor::PackageCacheLoader::PackageCacheLoader(std::shared_ptr<PackageCache> packageCache)
    : m_packageCache(std::move(packageCache))
{
    TU_ASSERT (m_packageCache != nullptr);
}

std::shared_ptr<zuri_distributor::PackageCache>
zuri_distributor::PackageCacheLoader::getPackageCache() const
{
    return m_packageCache;
}

tempo_utils::Result<std::filesystem::path>
zuri_distributor::PackageCacheLoader::findModule(
    const lyric_common::ModuleLocation &location,
    std::string_view dotSuffix) const
{
    if (!location.isValid() || location.getScheme() != "dev.zuri.pkg")
        return std::filesystem::path{};
    auto specifier = zuri_packager::PackageSpecifier::fromAuthority(location.getAuthority());
    if (!specifier.isValid())
        return std::filesystem::path{};

    Option<std::filesystem::path> pathOption;
    TU_ASSIGN_OR_RETURN (pathOption, m_packageCache->resolvePackage(specifier));
    if (pathOption.isEmpty())
        return std::filesystem::path{};

    auto modulesPath = pathOption.getValue() / "modules";
    auto modulePath = location.getPath().toFilesystemPath(modulesPath);
    modulePath.replace_extension(dotSuffix);

    if (modulePath.empty())
        return std::filesystem::path{};
    if (!std::filesystem::is_regular_file(modulePath))
        return std::filesystem::path{};
    return modulePath;
}

tempo_utils::Result<bool>
zuri_distributor::PackageCacheLoader::hasModule(const lyric_common::ModuleLocation &location) const
{
    std::filesystem::path absolutePath;
    TU_ASSIGN_OR_RETURN (absolutePath, findModule(location, lyric_common::kObjectFileDotSuffix));
    return !absolutePath.empty();
}

tempo_utils::Result<Option<lyric_object::LyricObject>>
zuri_distributor::PackageCacheLoader::loadModule(const lyric_common::ModuleLocation &location)
{
    std::filesystem::path absolutePath;
    TU_ASSIGN_OR_RETURN (absolutePath, findModule(location, lyric_common::kObjectFileDotSuffix));
    if (absolutePath.empty())
        return Option<lyric_object::LyricObject>();

    tempo_utils::FileReader reader(absolutePath.string());
    if (!reader.isValid())
        return reader.getStatus();
    auto bytes = reader.getBytes();

    // verify that file contents is a valid object
    if (!lyric_object::LyricObject::verify(std::span<const tu_uint8>(bytes->getData(), bytes->getSize())))
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "failed to verify object");

    // return platform-specific LyricObject
    TU_LOG_V << "loaded module at " << absolutePath;
    return Option(lyric_object::LyricObject(bytes));
}

tempo_utils::Result<Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>>
zuri_distributor::PackageCacheLoader::loadPlugin(
    const lyric_common::ModuleLocation &location,
    const lyric_object::PluginSpecifier &specifier)
{
    std::filesystem::path absolutePath;
    TU_ASSIGN_OR_RETURN (absolutePath, findModule(location, absl::StrCat(
        ".", tempo_utils::sharedLibraryPlatformId(), tempo_utils::sharedLibraryFileDotSuffix())));
    if (absolutePath.empty())
        return Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>();

    // attempt to load the plugin
    auto loader = std::make_shared<tempo_utils::LibraryLoader>(absolutePath, "native_init");
    if (!loader->isValid()) {
        auto status = loader->getStatus();
        // if plugin is not found then return empty option instead of status
        if (status.getStatusCode() == tempo_utils::StatusCode::kNotFound)
            return Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>();
        return status;
    }

    // cast raw pointer to native_init function pointer
    auto native_init = (lyric_runtime::NativeInitFunc) loader->symbolPointer();
    if (native_init == nullptr)
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "failed to retrieve native_init symbol from plugin {}", absolutePath.string());

    // retrieve the plugin interface
    auto *iface = native_init();
    if (iface == nullptr)
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "failed to retrieve interface for plugin {}", absolutePath.string());

    TU_LOG_V << "loaded plugin " << absolutePath;
    auto plugin = std::make_shared<const lyric_runtime::LibraryPlugin>(loader, iface);
    return Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>(plugin);
}
