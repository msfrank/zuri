
#include <tempo_utils/program_location.h>
#include <zuri_tooling/distribution.h>
#include <zuri_tooling/tooling_result.h>

zuri_tooling::Distribution::Distribution()
{
}

zuri_tooling::Distribution::Distribution(
    const std::filesystem::path &binDirectory,
    const std::filesystem::path &libDirectory,
    const std::filesystem::path &configDirectory)
    : m_priv(std::make_shared<Priv>(
        binDirectory,
        libDirectory,
        configDirectory))
{
}

zuri_tooling::Distribution::Distribution(const Distribution &other)
    : m_priv(other.m_priv)
{
}

bool
zuri_tooling::Distribution::isValid() const
{
    return m_priv != nullptr;
}

std::filesystem::path
zuri_tooling::Distribution::getBinDirectory() const
{
    if (m_priv)
        return m_priv->binDirectory;
    return {};
}

std::filesystem::path
zuri_tooling::Distribution::getLibDirectory() const
{
    if (m_priv)
        return m_priv->libDirectory;
    return {};
}

std::filesystem::path
zuri_tooling::Distribution::getConfigDirectory() const
{
    if (m_priv)
        return m_priv->configDirectory;
    return {};
}

inline std::filesystem::path resolve_path(
    const std::filesystem::path &p,
    const std::filesystem::path &base)
{
    if (p.is_absolute())
        return p;
    auto full_path = base / p;
    return full_path.lexically_normal();
}

tempo_utils::Result<zuri_tooling::Distribution>
zuri_tooling::Distribution::open()
{
    std::filesystem::path binDirectory;
    TU_ASSIGN_OR_RETURN (binDirectory, tempo_utils::get_program_directory());
    if (!std::filesystem::exists(binDirectory))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing distribution bin directory {}", binDirectory.string());

    auto libDirectory = resolve_path(std::filesystem::path{RELATIVE_LIB_DIR}, binDirectory);
    if (!std::filesystem::exists(libDirectory))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing distribution lib directory {}", libDirectory.string());

    auto configDirectory = resolve_path(std::filesystem::path{RELATIVE_CONFIG_DIR}, binDirectory);
    if (!std::filesystem::exists(configDirectory))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing distribution config directory {}", configDirectory.string());

    return Distribution(binDirectory, libDirectory, configDirectory);
}

tempo_utils::Result<zuri_tooling::Distribution>
zuri_tooling::Distribution::find(const std::filesystem::path &programLocation)
{
    std::filesystem::path binDirectory;

    if (std::filesystem::is_symlink(programLocation)) {
        std::error_code ec;
        auto linkTarget = std::filesystem::read_symlink(programLocation, ec);
        if (ec)
            return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
                "failed to resolve symlink {}: {}", programLocation.string(), ec.message());
        if (!std::filesystem::is_regular_file(linkTarget))
            return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
                "symlink target {} is not a regular file", linkTarget.string());
        binDirectory = linkTarget.parent_path();
    } else {
        binDirectory = programLocation.parent_path();
    }

    auto libDirectory = resolve_path(std::filesystem::path{RELATIVE_LIB_DIR}, binDirectory);
    if (!std::filesystem::exists(libDirectory))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing distribution lib directory {}", libDirectory.string());

    auto configDirectory = resolve_path(std::filesystem::path{RELATIVE_CONFIG_DIR}, binDirectory);
    if (!std::filesystem::exists(configDirectory))
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "missing distribution config directory {}", configDirectory.string());

    return Distribution(binDirectory, libDirectory, configDirectory);
}
