
#include <lyric_common/common_types.h>
#include <lyric_common/plugin.h>
#include <lyric_runtime/library_plugin.h>
#include <tempo_utils/file_reader.h>
#include <tempo_utils/library_loader.h>
#include <tempo_utils/platform.h>
#include <zuri_distributor/distributor_result.h>
#include <zuri_distributor/package_requirements_loader.h>

zuri_distributor::PackageRequirementsLoader::PackageRequirementsLoader(
    const absl::flat_hash_map<std::string, std::filesystem::path> &importPackages)
        : m_importPackages(importPackages)
{
}

tempo_utils::Result<std::filesystem::path>
zuri_distributor::PackageRequirementsLoader::findModule(
    const lyric_common::ModuleLocation &location,
    std::string_view dotSuffix) const
{
    if (!location.isValid() || !location.getScheme().empty())
        return std::filesystem::path{};

    auto authority = location.getAuthority();
    if (location.getAuthority().isEmpty())
        return std::filesystem::path{};
    if (authority.hasCredentials() || authority.hasPort())
        return std::filesystem::path{};

    auto importName = authority.getHost();
    auto entry = m_importPackages.find(importName);
    if (entry == m_importPackages.cend())
        return std::filesystem::path{};

    auto modulesPath = entry->second / "modules";
    auto modulePath = location.getPath().toFilesystemPath(modulesPath);
    modulePath.replace_extension(dotSuffix);

    if (modulePath.empty())
        return std::filesystem::path{};
    if (!std::filesystem::is_regular_file(modulePath))
        return std::filesystem::path{};
    return modulePath;
}

tempo_utils::Result<bool>
zuri_distributor::PackageRequirementsLoader::hasModule(const lyric_common::ModuleLocation &location) const
{
    std::filesystem::path absolutePath;
    TU_ASSIGN_OR_RETURN (absolutePath, findModule(location, lyric_common::kObjectFileDotSuffix));
    return !absolutePath.empty();
}

tempo_utils::Result<Option<lyric_object::LyricObject>>
zuri_distributor::PackageRequirementsLoader::loadModule(
    const lyric_common::ModuleLocation &location)
{
    std::filesystem::path absolutePath;
    TU_ASSIGN_OR_RETURN (absolutePath, findModule(location, lyric_common::kObjectFileDotSuffix));
    if (absolutePath.empty())
        return Option<lyric_object::LyricObject>();

    tempo_utils::FileReader reader(absolutePath.string());
    if (!reader.isValid())
        return reader.getStatus();
    auto bytes = reader.getBytes();

    // verify that file contents is a valid assembly
    if (!lyric_object::LyricObject::verify(std::span<const tu_uint8>(bytes->getData(), bytes->getSize())))
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "failed to verify object");

    // return platform-specific LyricAssembly
    TU_LOG_INFO << "loaded module at " << absolutePath;
    return Option(lyric_object::LyricObject(bytes));
}

tempo_utils::Result<Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>>
zuri_distributor::PackageRequirementsLoader::loadPlugin(
    const lyric_common::ModuleLocation &location, const lyric_object::PluginSpecifier &specifier)
{
    std::filesystem::path absolutePath;
    TU_ASSIGN_OR_RETURN (absolutePath, findModule(location, absl::StrCat(
        ".", tempo_utils::sharedLibraryPlatformId(), tempo_utils::sharedLibraryFileDotSuffix())));
    if (absolutePath.empty())
        return Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>();

    auto pluginName = lyric_common::pluginFilename(absolutePath.stem().string());
    absolutePath.replace_filename(pluginName);

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

    TU_LOG_INFO << "loaded plugin " << absolutePath;
    auto plugin = std::make_shared<const lyric_runtime::LibraryPlugin>(loader, iface);
    return Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>(plugin);
}