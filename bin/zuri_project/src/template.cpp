
#include <zuri_project/project_result.h>
#include <zuri_project/template.h>

zuri_project::Template::Template()
{
}

zuri_project::Template::Template(
    const std::filesystem::path &templateConfigFile,
    const std::filesystem::path &templateDirectory,
    const std::filesystem::path &targetConfigTemplateFile)
    : m_priv(std::make_shared<Priv>(
        templateConfigFile,
        templateDirectory,
        targetConfigTemplateFile))
{
    TU_ASSERT (!m_priv->templateConfigFile.empty());
    TU_ASSERT (!m_priv->templateDirectory.empty());
    TU_ASSERT (!m_priv->targetConfigTemplateFile.empty());
}

zuri_project::Template::Template(const Template &other)
    : m_priv(other.m_priv)
{
}

bool
zuri_project::Template::isValid() const
{
    return m_priv != nullptr;
}

std::filesystem::path
zuri_project::Template::getTemplateConfigFile() const
{
    if (m_priv != nullptr)
        return m_priv->templateConfigFile;
    return {};
}

std::filesystem::path
zuri_project::Template::getTemplateDirectory() const
{
    if (m_priv != nullptr)
        return m_priv->templateDirectory;
    return {};
}

std::filesystem::path
zuri_project::Template::getTargetConfigTemplateFile() const
{
    if (m_priv != nullptr)
        return m_priv->targetConfigTemplateFile;
    return {};
}

tempo_utils::Result<zuri_project::Template>
zuri_project::Template::open(const std::filesystem::path &templateDirectoryOrConfigFile)
{
    std::filesystem::path templateConfigFile;
    std::filesystem::path templateDirectory;

    if (std::filesystem::is_regular_file(templateDirectoryOrConfigFile)) {
        templateConfigFile = templateDirectoryOrConfigFile;
        templateDirectory = templateConfigFile.parent_path();
    } else if (std::filesystem::is_directory(templateDirectoryOrConfigFile)) {
        templateDirectory = templateDirectoryOrConfigFile;
        templateConfigFile = templateDirectory / kTemplateConfigName;
    } else {
        return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
            "zuri template not found at {}", templateDirectoryOrConfigFile.string());
    }

    if (!std::filesystem::exists(templateConfigFile))
        return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
            "missing zuri template config file {}", templateConfigFile.string());

    auto targetConfigTemplateFile = templateDirectory / kTargetConfigTemplateName;
    if (!std::filesystem::exists(targetConfigTemplateFile))
        return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
            "missing target config template file {}", targetConfigTemplateFile.string());

    return Template(templateConfigFile, templateDirectory, targetConfigTemplateFile);
}