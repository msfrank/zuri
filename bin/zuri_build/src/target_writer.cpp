
#include <filesystem>

#include <LIEF/MachO.hpp>
#include <LIEF/ELF.hpp>

#include <lyric_build/build_attrs.h>
#include <lyric_build/build_result.h>
#include <lyric_common/common_types.h>
#include <lyric_object/lyric_object.h>
#include <tempo_config/config_builder.h>
#include <tempo_config/parse_config.h>
#include <tempo_utils/directory_maker.h>
#include <tempo_utils/log_message.h>
#include <tempo_utils/memory_bytes.h>
#include <zuri_build/target_writer.h>

#include "zuri_packager/packaging_conversions.h"

zuri_build::TargetWriter::TargetWriter(
    std::shared_ptr<zuri_distributor::RuntimeEnvironment> runtimeEnvironment,
    const std::filesystem::path &installRoot,
    const zuri_packager::PackageSpecifier &specifier)
    : m_runtimeEnvironment(runtimeEnvironment),
      m_installRoot(installRoot),
      m_specifier(specifier),
      m_priv(std::make_unique<Priv>())
{
    TU_ASSERT (m_runtimeEnvironment != nullptr);
    TU_ASSERT (!m_installRoot.empty());
    TU_ASSERT (m_specifier.isValid());
}

tempo_utils::Status
zuri_build::TargetWriter::configure()
{
    if (m_priv == nullptr)
        return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kBuildInvariant,
            "target writer is finished");
    if (m_priv->packageWriter != nullptr)
        return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kBuildInvariant,
            "target writer is already configured");

    zuri_packager::PackageWriterOptions options;
    options.installRoot = m_installRoot;
    options.overwriteFile = true;
    auto packageWriter = std::make_unique<zuri_packager::PackageWriter>(m_specifier, options);
    TU_RETURN_IF_NOT_OK (packageWriter->configure());
    m_priv->packageWriter = std::move(packageWriter);

    return {};
}

void
zuri_build::TargetWriter::setDescription(std::string_view description)
{
    if (m_priv == nullptr)
        return;
    m_priv->description = description;
}

void
zuri_build::TargetWriter::setOwner(std::string_view owner)
{
    if (m_priv == nullptr)
        return;
    m_priv->owner = owner;
}

void
zuri_build::TargetWriter::setHomepage(std::string_view homepage)
{
    if (m_priv == nullptr)
        return;
    m_priv->homepage = homepage;
}

void
zuri_build::TargetWriter::setLicense(std::string_view license)
{
    if (m_priv == nullptr)
        return;
    m_priv->license = license;
}

void
zuri_build::TargetWriter::setProgramMain(const lyric_common::ModuleLocation &programMain)
{
    if (m_priv == nullptr)
        return;
    m_priv->programMain = programMain;
}

tempo_utils::Status
zuri_build::TargetWriter::addRequirement(const zuri_packager::PackageSpecifier &specifier)
{
    if (!specifier.isValid())
        return zuri_packager::PackagerStatus::forCondition(zuri_packager::PackagerCondition::kPackagerInvariant,
            "invalid requirement");
    if (m_priv == nullptr)
        return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kBuildInvariant,
            "target writer is finished");

    auto id = specifier.getPackageId();
    auto version = specifier.getPackageVersion();
    auto entry = m_priv->requirements.find(id);
    if (entry != m_priv->requirements.cend()) {
        if (version > entry->second) {
            entry->second = version;
        }
    } else {
        m_priv->requirements[id] = version;
    }

    return {};
}

inline std::string
make_relative_rpath(
    const tempo_utils::UrlPath &pluginFile,
    const tempo_utils::UrlPath &libDirectory,
    std::string_view originName)
{
    auto pluginFilePath = pluginFile.toFilesystemPath("/");
    pluginFilePath.replace_filename(originName);
    auto libDirectoryPath = libDirectory.toFilesystemPath("/");
    return pluginFilePath.lexically_relative(libDirectoryPath);
}

tempo_utils::Result<
    std::pair<
        std::shared_ptr<const tempo_utils::ImmutableBytes>,
        zuri_build::TargetWriter::PluginInfo>>
zuri_build::TargetWriter::rewritePlugin(const tempo_utils::UrlPath &path, std::span<const tu_uint8> content)
{
    std::filesystem::path pluginName(path.lastView());
    auto pluginExtension = pluginName.extension();
    std::vector pluginData(content.begin(), content.end());

    auto distributionLibPath = tempo_utils::UrlPath::fromString("/distribution/lib");
    auto libPath = tempo_utils::UrlPath::fromString("/lib");

    PluginInfo pluginInfo;
    pluginInfo.path = path;

    if (pluginExtension == ".dylib") {
        // load the Mach-O binary
        auto fatBinary = LIEF::MachO::Parser::parse(pluginData);
        auto dso = fatBinary->at(0);

        // record all dependencies
        for (const auto &library : dso->libraries()) {
            pluginInfo.libraries.insert(library.name());
        }
        // clear existing rpath commands
        while (dso->has_rpath()) {
            dso->remove(*dso->rpath());
        }
        // for (auto &rpath : dso->rpaths()) {
        //     dso->remove(rpath);
        // }

        // add the rpath for the distribution/lib directory
        auto distributionLibRpathCommand = LIEF::MachO::RPathCommand::create(
            make_relative_rpath(path, distributionLibPath, "@loader_path"));
        dso->add(*distributionLibRpathCommand);

        // add the rpath for the lib directory
        auto libRpathCommand = LIEF::MachO::RPathCommand::create(
            make_relative_rpath(path, libPath, "@loader_path"));
        dso->add(*libRpathCommand);

        // write the new content
        std::vector<tu_uint8> rewrittenContent;
        LIEF::MachO::Builder::write(*fatBinary, rewrittenContent);
        auto rewrittenBytes = std::static_pointer_cast<const tempo_utils::ImmutableBytes>(
            tempo_utils::MemoryBytes::create(std::move(rewrittenContent)));

        std::pair p(rewrittenBytes, pluginInfo);

        return p;
    }

    if (pluginExtension == ".so") {

    }

    return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kBuildInvariant,
        "unsupported DSO type");
}

using perms = std::filesystem::perms;

tempo_utils::Status
zuri_build::TargetWriter::writeModule(
    const tempo_utils::UrlPath &modulePath,
    const lyric_build::LyricMetadata &metadata,
    std::shared_ptr<const tempo_utils::ImmutableBytes> content)
{
    if (m_priv == nullptr)
        return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kBuildInvariant,
            "target writer is finished");
    if (m_priv->packageWriter == nullptr)
        return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kBuildInvariant,
            "target writer is not configured");

    auto modulesRoot = tempo_utils::UrlPath::fromString("/modules");
    auto fullModulePath = modulesRoot.traverse(modulePath.toRelative());

    std::string contentType;
    TU_RETURN_IF_NOT_OK (metadata.parseAttr(lyric_build::kLyricBuildContentType, contentType));

    if (contentType == lyric_common::kObjectContentType) {
        lyric_object::LyricObject object(content);
        for (int i = 0; i < object.numImports(); i++) {
            auto import = object.getImport(i);
            auto location = import.getImportLocation();

            // ignore relative imports
            if (location.isRelative())
                continue;

            if (import.isSystemBootstrap()) {
                // TODO: process bootstrap imports
            } else if (location.getScheme() == "dev.zuri.pkg") {
                auto specifier = zuri_packager::PackageSpecifier::fromAuthority(location.getAuthority());
                TU_RETURN_IF_NOT_OK (addRequirement(specifier));
            } else {
                return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kBuildInvariant,
                    "unhandled module scheme '{}' for import {}", location.getScheme(), location.toString());
            }
        }
    } else if (contentType == lyric_common::kPluginContentType) {
        std::pair<std::shared_ptr<const tempo_utils::ImmutableBytes>,PluginInfo> p;
        TU_ASSIGN_OR_RETURN (p, rewritePlugin(fullModulePath, content->getSpan()));
        m_priv->plugins[p.second.path] = std::move(p.second);
        content = std::move(p.first);
    } else {
        return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kBuildInvariant,
            "unhandled module content '{}' for {}", contentType, fullModulePath.toString());
    }

    TU_LOG_V << "writing " << fullModulePath;

    auto parentPath = fullModulePath.getInit();
    TU_RETURN_IF_STATUS (m_priv->packageWriter->makeDirectory(parentPath, true));
    TU_RETURN_IF_STATUS (m_priv->packageWriter->putFile(fullModulePath, content));

    return {};
}

tempo_utils::Status
zuri_build::TargetWriter::determineLibrariesNeeded()
{
    // build map of libraries available based on system, distribution, and package requirements
    absl::flat_hash_map<std::string,std::string> librariesAvailable;

    // add explicitly declared system libraries
    for (const auto &libraryName : m_systemLibNames) {
        if (librariesAvailable.contains(libraryName))
            return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kBuildInvariant,
                "duplicate library {}; already provided by $SYSTEM$", libraryName);
        librariesAvailable[libraryName] = "$SYSTEM$";
    }

    // add libraries from distribution lib directories
    for (const auto &distributionLibDirectory : m_distributionLibDirectories) {
        std::filesystem::directory_iterator it(distributionLibDirectory);
        for (const auto &entry : it) {
            if (!entry.is_regular_file())
                continue;
            auto libraryName = entry.path().filename().string();
            if (!libraryName.starts_with("lib"))
                continue;
            if (librariesAvailable.contains(libraryName))
                return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kBuildInvariant,
                    "duplicate library {}; already provided by {}",
                    libraryName, librariesAvailable.at(libraryName));
            librariesAvailable[libraryName] = "$DISTRIBUTION$";
        }
    }

    // add libraries which are provided by package requirements
    for (const auto &entry : m_priv->requirements) {
        auto specifier = zuri_packager::PackageSpecifier(entry.first, entry.second);

        Option<tempo_config::ConfigMap> packageConfigOption;
        TU_ASSIGN_OR_RETURN (packageConfigOption, m_runtimeEnvironment->describePackage(specifier));
        if (packageConfigOption.isEmpty())
            return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kBuildInvariant,
                "failed to read package.config for {}", specifier.toString());
        auto packageConfig = packageConfigOption.getValue();

        zuri_packager::LibrariesProvidedParser librariesProvidedParser(zuri_packager::LibrariesProvided{});
        zuri_packager::LibrariesProvided librariesProvided;
        TU_RETURN_IF_NOT_OK (tempo_config::parse_config(librariesProvided, librariesProvidedParser,
            packageConfig, "librariesProvided"));

        auto librarySource = specifier.toString();
        for (auto it = librariesProvided.providedBegin(); it != librariesProvided.providedEnd(); ++it) {
            const auto &libraryName = *it;
            if (librariesAvailable.contains(libraryName))
                return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kBuildInvariant,
                    "duplicate library {}; provided in both {} and {}",
                    libraryName, librariesAvailable.at(libraryName), librarySource);
            librariesAvailable[libraryName] = librarySource;
        }
    }

    // verify that each library dependency for every plugin is satisfied
    for (const auto &entry : m_priv->plugins) {
        for (const auto &libraryName : entry.second.libraries) {
            auto available = librariesAvailable.find(libraryName);
            if (available == librariesAvailable.cend()) {
                // FIXME
                TU_LOG_V << "missing library " << libraryName << " for plugin " << entry.second.path.toString() ;
                continue;
                //return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kBuildInvariant,
                //    "missing library {} for plugin {}",
                //    libraryName, entry.second.path.toString());
            }
            auto &librarySource = available->second;
            if (librarySource == "$SYSTEM$") {
                m_priv->librariesNeeded.addSystemLibrary(libraryName);
            } else if (librarySource == "$DISTRIBUTION$") {
                m_priv->librariesNeeded.addDistributionLibrary(libraryName);
            } else {
                auto packageId = zuri_packager::PackageId::fromString(librarySource);
                m_priv->librariesNeeded.addPackageLibrary(packageId, libraryName);
            }
        }
    }

    return {};
}

tempo_utils::Status
zuri_build::TargetWriter::writePackageConfig()
{
    auto rootBuilder = tempo_config::startMap()
        .put("name", tempo_config::valueNode(m_specifier.getPackageName()))
        .put("version", tempo_config::valueNode(m_specifier.getVersionString()))
        .put("domain", tempo_config::valueNode(m_specifier.getPackageDomain()));

    if (!m_priv->description.empty()) {
        rootBuilder = rootBuilder.put("description", tempo_config::valueNode(m_priv->description));
    }
    if (!m_priv->owner.empty()) {
        rootBuilder = rootBuilder.put("owner", tempo_config::valueNode(m_priv->owner));
    }
    if (!m_priv->homepage.empty()) {
        rootBuilder = rootBuilder.put("homepage", tempo_config::valueNode(m_priv->homepage));
    }
    if (!m_priv->license.empty()) {
        rootBuilder = rootBuilder.put("license", tempo_config::valueNode(m_priv->license));
    }
    if (m_priv->programMain.isValid()) {
        rootBuilder = rootBuilder.put("programMain", tempo_config::valueNode(m_priv->programMain.toString()));
    }
    if (!m_priv->requirements.empty()) {
        auto requirementsBuilder = tempo_config::startMap();
        for (const auto &req : m_priv->requirements) {
            requirementsBuilder = requirementsBuilder.put(
                req.first.toString(),
                tempo_config::valueNode(req.second.toString()));
        }
        rootBuilder = rootBuilder.put("requirements", requirementsBuilder.buildNode());
    }
    if (!m_priv->librariesNeeded.isEmpty()) {
        absl::btree_map<std::string,tempo_config::ConfigNode> neededEntries;
        for (auto it = m_priv->librariesNeeded.neededBegin(); it != m_priv->librariesNeeded.neededEnd(); ++it) {
            std::vector<tempo_config::ConfigNode> libraryNames;
            for (const auto &libraryName : *it->second) {
                libraryNames.push_back(tempo_config::valueNode(libraryName));
            }
            neededEntries[it->first] = tempo_config::ConfigSeq(std::move(libraryNames));
        }
        rootBuilder = rootBuilder.put("librariesNeeded", tempo_config::ConfigMap(std::move(neededEntries)));
    }

    auto packageConfig = rootBuilder.buildMap();
    m_priv->packageWriter->setPackageConfig(packageConfig);

    return {};
}

tempo_utils::Result<std::filesystem::path>
zuri_build::TargetWriter::writeTarget()
{
    if (m_priv == nullptr)
        return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kBuildInvariant,
            "target writer is finished");
    if (m_priv->packageWriter == nullptr)
        return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kBuildInvariant,
            "target writer is not configured");

    TU_RETURN_IF_NOT_OK (determineLibrariesNeeded());

    TU_RETURN_IF_NOT_OK (writePackageConfig());

    std::filesystem::path packagePath;
    TU_ASSIGN_OR_RETURN (packagePath, m_priv->packageWriter->writePackage());

    TU_LOG_V << "writing package to " << packagePath;

    m_priv.reset();

    return packagePath;
}