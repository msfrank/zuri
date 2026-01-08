
#include <tempo_utils/user_home.h>
#include <zuri_tooling/home.h>

#include "zuri_tooling/tooling_result.h"
#include "zuri_tooling/zuri_config.h"

static const char *kHomeDirectoryName = ".zuri-" PROJECT_VERSION_MAJOR;

zuri_tooling::Home::Home()
{
}

zuri_tooling::Home::Home(
    const std::filesystem::path &homeDirectory,
    const std::filesystem::path &packagesDirectory,
    const std::filesystem::path &environmentsDirectory,
    const std::filesystem::path &configDirectory,
    const std::filesystem::path &vendorConfigDirectory)
    : m_priv(std::make_shared<Priv>(
        homeDirectory,
        packagesDirectory,
        environmentsDirectory,
        configDirectory,
        vendorConfigDirectory))
{
}

zuri_tooling::Home::Home(const Home &other)
    : m_priv(other.m_priv)
{
}

bool
zuri_tooling::Home::isValid() const
{
    return m_priv != nullptr;
}

std::filesystem::path
zuri_tooling::Home::getHomeDirectory() const
{
    if (m_priv)
        return m_priv->homeDirectory;
    return {};
}

std::filesystem::path
zuri_tooling::Home::getPackagesDirectory() const
{
    if (m_priv)
        return m_priv->packagesDirectory;
    return {};
}

std::filesystem::path
zuri_tooling::Home::getEnvironmentsDirectory() const
{
    if (m_priv)
        return m_priv->environmentsDirectory;
    return {};
}

std::filesystem::path
zuri_tooling::Home::getConfigDirectory() const
{
    if (m_priv)
        return m_priv->configDirectory;
    return {};
}

std::filesystem::path
zuri_tooling::Home::getVendorConfigDirectory() const
{
    if (m_priv)
        return m_priv->vendorConfigDirectory;
    return {};
}

inline std::filesystem::path
determine_home_directory()
{
    const auto *value = std::getenv(zuri_tooling::kEnvHomeName);
    return value? std::filesystem::path{value}
        : tempo_utils::get_user_home_directory() / kHomeDirectoryName;
}

tempo_utils::Result<zuri_tooling::Home>
zuri_tooling::Home::openOrCreate(const std::filesystem::path &homeDirectory_)
{
    auto homeDirectory = homeDirectory_.empty()? determine_home_directory() : homeDirectory_;
    if (std::filesystem::exists(homeDirectory))
        return open(homeDirectory, /* ignoreMissing= */ false);

    std::error_code ec;
    std::filesystem::create_directory(homeDirectory, ec);
    if (ec)
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "failed to create zuri home directory {}", homeDirectory.string());

    auto packagesDirectory = homeDirectory / "packages";
    std::filesystem::create_directory(packagesDirectory, ec);
    if (ec)
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "failed to create home packages directory {}", packagesDirectory.string());

    auto environmentsDirectory = homeDirectory / "environments";
    std::filesystem::create_directory(environmentsDirectory, ec);
    if (ec)
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "failed to create home environments directory {}", environmentsDirectory.string());

    auto configDirectory = homeDirectory / "config";
    std::filesystem::create_directory(configDirectory, ec);
    if (ec)
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "failed to create home config directory {}", configDirectory.string());

    auto vendorConfigDirectory = homeDirectory / "vendor-config";
    std::filesystem::create_directory(vendorConfigDirectory, ec);
    if (ec)
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "failed to create home vendor-config directory {}", vendorConfigDirectory.string());

    return Home(homeDirectory, packagesDirectory, environmentsDirectory, configDirectory, vendorConfigDirectory);
}

tempo_utils::Result<zuri_tooling::Home>
zuri_tooling::Home::open(const std::filesystem::path &homeDirectory, bool ignoreMissing)
{
    if (!std::filesystem::exists(homeDirectory)) {
        if (ignoreMissing)
            return Home{};
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing zuri home directory {}", homeDirectory.string());
    }

    auto packagesDirectory = homeDirectory / "packages";
    if (!std::filesystem::exists(packagesDirectory))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing home packages directory {}", packagesDirectory.string());

    auto environmentsDirectory = homeDirectory / "environments";
    if (!std::filesystem::exists(environmentsDirectory))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing home environments directory {}", environmentsDirectory.string());

    auto configDirectory = homeDirectory / "config";
    if (!std::filesystem::exists(packagesDirectory))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing home config directory {}", packagesDirectory.string());

    auto vendorConfigDirectory = homeDirectory / "vendor-config";
    if (!std::filesystem::exists(packagesDirectory))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing home vendor-config directory {}", packagesDirectory.string());

    return Home(homeDirectory, packagesDirectory, environmentsDirectory, configDirectory, vendorConfigDirectory);
}

tempo_utils::Result<zuri_tooling::Home>
zuri_tooling::Home::open(bool ignoreMissing)
{
    return open(determine_home_directory(), ignoreMissing);
}

const char *
zuri_tooling::homeDirectoryName()
{
    return kHomeDirectoryName;
}
