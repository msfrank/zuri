
#include <lyric_build/build_result.h>
#include <tempo_config/config_builder.h>
#include <tempo_config/config_serde.h>
#include <tempo_utils/directory_maker.h>
#include <tempo_utils/file_writer.h>
#include <tempo_utils/log_message.h>
#include <tempo_utils/memory_bytes.h>
#include <zuri_build/target_writer.h>

TargetWriter::TargetWriter(
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
TargetWriter::configure()
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
TargetWriter::setDescription(std::string_view description)
{
    if (m_priv == nullptr)
        return;
    m_priv->description = description;
}

void
TargetWriter::setOwner(std::string_view owner)
{
    if (m_priv == nullptr)
        return;
    m_priv->owner = owner;
}

void
TargetWriter::setHomepage(std::string_view homepage)
{
    if (m_priv == nullptr)
        return;
    m_priv->homepage = homepage;
}

void

TargetWriter::setLicense(std::string_view license)
{
    if (m_priv == nullptr)
        return;
    m_priv->license = license;
}

tempo_utils::Status
TargetWriter::addDependency(const zuri_packager::PackageDependency &dependency)
{
    if (!dependency.isValid())
        return zuri_packager::PackageStatus::forCondition(zuri_packager::PackageCondition::kPackageInvariant,
            "invalid dependency");
    if (m_priv == nullptr)
        return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kBuildInvariant,
            "target writer is finished");

    auto depId = absl::StrCat(dependency.getName(), "@", dependency.getDomain());
    if (m_priv->dependencies.contains(depId))
        return zuri_packager::PackageStatus::forCondition(zuri_packager::PackageCondition::kPackageInvariant,
            "dependency already exists for '{}'", depId);
    m_priv->dependencies[depId] = dependency;

    return {};
}

using perms = std::filesystem::perms;

tempo_utils::Status
TargetWriter::writeModule(
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

    // auto parentPerms = perms::owner_all
    //     | perms::group_read
    //     | perms::group_exec
    //     | perms::others_read
    //     | perms::others_exec;

    auto parentPath = fullModulePath.getInit();
    zuri_packager::EntryAddress parentEntry;
    TU_ASSIGN_OR_RETURN (parentEntry, m_priv->packageWriter->makeDirectory(parentPath, true));

    // auto modulePerms = perms::owner_read
    //     | perms::owner_write
    //     | perms::group_read
    //     | perms::others_read;

    zuri_packager::EntryAddress moduleEntry;
    TU_ASSIGN_OR_RETURN (moduleEntry, m_priv->packageWriter->putFile(fullModulePath, content));

    return {};
}

tempo_utils::Status
TargetWriter::writePackageConfig()
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

    if (!m_priv->dependencies.empty()) {
        auto dependenciesBuilder = tempo_config::startMap();
        for (const auto &dep : m_priv->dependencies) {
            dependenciesBuilder = dependenciesBuilder.put(dep.first, dep.second.toNode());
        }
        rootBuilder = rootBuilder.put("dependencies", dependenciesBuilder.buildNode());
    }

    auto packageConfig = rootBuilder.buildMap();
    m_priv->packageWriter->setPackageConfig(packageConfig);

    return {};
}

tempo_utils::Result<std::filesystem::path>
TargetWriter::writeTarget()
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

    m_priv.reset();

    return packagePath;
}