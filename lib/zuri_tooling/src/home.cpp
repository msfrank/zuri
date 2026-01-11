
#include <tempo_utils/user_home.h>
#include <zuri_tooling/home.h>
#include <zuri_tooling/tooling_result.h>

static const char *kHomeDirectoryName = ".zuri-" PROJECT_VERSION_MAJOR;

zuri_tooling::Home::Home()
{
}

zuri_tooling::Home::Home(
    const std::filesystem::path &homeDirectory,
    const std::filesystem::path &configDirectory,
    const std::filesystem::path &cacheDirectory)
    : m_priv(std::make_shared<Priv>(
        homeDirectory,
        configDirectory,
        cacheDirectory))
{
    TU_ASSERT (!m_priv->homeDirectory.empty());
    TU_ASSERT (!m_priv->configDirectory.empty());
    TU_ASSERT (!m_priv->cacheDirectory.empty());
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
zuri_tooling::Home::getConfigDirectory() const
{
    if (m_priv)
        return m_priv->configDirectory;
    return {};
}

std::filesystem::path
zuri_tooling::Home::getCacheDirectory() const
{
    if (m_priv)
        return m_priv->cacheDirectory;
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

    auto configDirectory = homeDirectory / "config";
    std::filesystem::create_directory(configDirectory, ec);
    if (ec)
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "failed to create home config directory {}", configDirectory.string());

    auto cacheDirectory = homeDirectory / "cache";
    std::filesystem::create_directory(cacheDirectory, ec);
    if (ec)
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "failed to create home cache directory {}", cacheDirectory.string());

    return Home(homeDirectory, configDirectory, cacheDirectory);
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

    auto configDirectory = homeDirectory / "config";
    if (!std::filesystem::exists(configDirectory))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing home config directory {}", configDirectory.string());

    auto cacheDirectory = homeDirectory / "cache";
    if (!std::filesystem::exists(cacheDirectory))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing home cache directory {}", cacheDirectory.string());

    return Home(homeDirectory, configDirectory, cacheDirectory);
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
