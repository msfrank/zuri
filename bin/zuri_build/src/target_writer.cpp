
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_result.h>
#include <lyric_common/common_types.h>
#include <lyric_object/lyric_object.h>
#include <tempo_config/config_builder.h>
#include <tempo_utils/directory_maker.h>
#include <tempo_utils/log_message.h>
#include <tempo_utils/memory_bytes.h>
#include <zuri_build/target_writer.h>

zuri_build::TargetWriter::TargetWriter(
    const std::filesystem::path &installRoot,
    const zuri_packager::PackageSpecifier &specifier)
    : m_installRoot(installRoot),
      m_specifier(specifier),
      m_priv(std::make_unique<Priv>())
{
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

    std::string contentType;
    TU_RETURN_IF_NOT_OK (metadata.getMetadata().parseAttr(lyric_build::kLyricBuildContentType, contentType));

    if (contentType == lyric_common::kObjectContentType) {
        lyric_object::LyricObject object(content);
        auto root = object.getObject();
        for (int i = 0; i < root.numImports(); i++) {
            auto import = root.getImport(i);
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
    }

    auto modulesRoot = tempo_utils::UrlPath::fromString("/modules");
    auto fullModulePath = modulesRoot.traverse(modulePath.toRelative());
    TU_LOG_V << "writing module " << fullModulePath;

    auto parentPath = fullModulePath.getInit();
    zuri_packager::EntryAddress parentEntry;
    TU_ASSIGN_OR_RETURN (parentEntry, m_priv->packageWriter->makeDirectory(parentPath, true));

    zuri_packager::EntryAddress moduleEntry;
    TU_ASSIGN_OR_RETURN (moduleEntry, m_priv->packageWriter->putFile(fullModulePath, content));

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

    TU_RETURN_IF_NOT_OK (writePackageConfig());

    std::filesystem::path packagePath;
    TU_ASSIGN_OR_RETURN (packagePath, m_priv->packageWriter->writePackage());

    TU_LOG_V << "writing package to " << packagePath;

    m_priv.reset();

    return packagePath;
}