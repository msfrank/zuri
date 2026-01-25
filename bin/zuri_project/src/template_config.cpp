
#include <absl/strings/match.h>
#include <tempo_config/abstract_converter.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/enum_conversions.h>
#include <tempo_config/parse_config.h>
#include <zuri_project/project_conversions.h>
#include <zuri_project/project_result.h>
#include <zuri_project/template_config.h>

#include "zuri_packager/packaging_conversions.h"

zuri_project::TemplateConfig::TemplateConfig(const Template &tmpl, const tempo_config::ConfigMap &configMap)
    : m_template(tmpl),
      m_configMap(configMap)
{
    TU_ASSERT (m_template.isValid());
}

tempo_utils::Result<std::shared_ptr<zuri_project::TemplateConfig>>
zuri_project::TemplateConfig::load(const Template &tmpl)
{
    auto templateConfigFile = tmpl.getTemplateConfigFile();

    tempo_config::ConfigMap configMap;
    TU_ASSIGN_OR_RETURN (configMap, tempo_config::read_config_map_file(tmpl.getTemplateConfigFile()));

    auto templateConfig = std::shared_ptr<TemplateConfig>(new TemplateConfig(tmpl, configMap));
    TU_RETURN_IF_NOT_OK (templateConfig->configure());

    return templateConfig;
}

tempo_utils::Status
zuri_project::TemplateConfig::configure()
{
    tempo_config::StringParser nameParser;
    tempo_config::PathParser contentRootParser(m_template.getTemplateDirectory());
    tempo_config::StringParser parameterNameParser;
    ParameterEntryParser parameterEntryParser;
    tempo_config::SharedPtrConstTParser sharedConstParameterEntryParser(&parameterEntryParser);
    tempo_config::MapKVParser parameterEntriesParser(&parameterNameParser, &sharedConstParameterEntryParser, {});
    zuri_packager::PackageIdParser packageIdParser;
    zuri_packager::PackageVersionParser packageVersionParser;
    tempo_config::MapKVParser requirementEntriesParser(&packageIdParser, &packageVersionParser, {});

    // parse the required template name
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(m_name, nameParser,
        m_configMap, "name"));

    // parse the content root path, and make path absolute relative to the template directory if needed
    std::filesystem::path contentRoot;
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(contentRoot, contentRootParser,
        m_configMap, "contentRoot"));
    if (contentRoot.is_relative()) {
        contentRoot = m_template.getTemplateDirectory() / contentRoot;
    }
    m_contentRoot = std::move(contentRoot);

    // parse the optional template parameters map
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(m_parameterStore, parameterEntriesParser,
        m_configMap, "templateParameters"));

    // verify that parameter names are valid
    for (const auto &entry : m_parameterStore) {
        if (absl::StrContains(entry.first, "::"))
            return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
                "invalid template parameter name '{}; name cannot contain '::'", entry.first);
    }

    // parse the optional requirements map
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(m_requirements, requirementEntriesParser,
        m_configMap, "requirements"));

    return {};
}

zuri_project::Template
zuri_project::TemplateConfig::getTemplate() const
{
    return m_template;
}

std::string
zuri_project::TemplateConfig::getName() const
{
    return m_name;
}

std::filesystem::path
zuri_project::TemplateConfig::getContentRoot() const
{
    return m_contentRoot;
}

bool
zuri_project::TemplateConfig::hasParameter(std::string_view name) const
{
    return m_parameterStore.contains(name);
}

std::shared_ptr<const zuri_project::ParameterEntry>
zuri_project::TemplateConfig::getParameter(std::string_view name) const
{
    auto entry = m_parameterStore.find(name);
    if (entry != m_parameterStore.cend())
        return entry->second;
    return {};
}

absl::flat_hash_map<std::string,std::shared_ptr<const zuri_project::ParameterEntry>>::const_iterator
zuri_project::TemplateConfig::parametersBegin() const
{
    return m_parameterStore.cbegin();
}

absl::flat_hash_map<std::string,std::shared_ptr<const zuri_project::ParameterEntry>>::const_iterator
zuri_project::TemplateConfig::parametersEnd() const
{
    return m_parameterStore.cend();
}

int
zuri_project::TemplateConfig::numParameters() const
{
    return m_parameterStore.size();
}