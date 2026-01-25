
#include <absl/strings/match.h>
#include <mustache/mustache.hpp>

#include <tempo_utils/file_reader.h>
#include <zuri_project/project_result.h>
#include <zuri_project/template_processor.h>

using namespace kainjow;

struct zuri_project::TemplateProcessor::Priv {
    absl::flat_hash_map<std::string,std::shared_ptr<const ParameterEntry>> parameterStore;
    mustache::data templateContext;
    absl::flat_hash_set<std::string> missingArguments;
};

static tempo_utils::Status
build_mustache_data(const tempo_config::ConfigNode &node, mustache::data &data)
{
    switch (node.getNodeType()) {
        case tempo_config::ConfigNodeType::kNil: {
            data = {};
            return {};
        }
        case tempo_config::ConfigNodeType::kValue: {
            auto value = node.toValue();
            data = mustache::data{value.getValue()};
            return {};
        }
        case tempo_config::ConfigNodeType::kSeq: {
            auto seq = node.toSeq();

            mustache::list list;
            for (auto it = seq.seqBegin(); it != seq.seqEnd(); ++it) {
                mustache::data element;
                TU_RETURN_IF_NOT_OK (build_mustache_data(*it, element));
                list.push_back(std::move(element));
            }
            data = list;
            return {};
        }
        case tempo_config::ConfigNodeType::kMap: {
            auto map = node.toMap();

            mustache::object object;
            for (auto it = map.mapBegin(); it != map.mapEnd(); ++it) {
                const auto &key = it->first;
                mustache::data value;
                TU_RETURN_IF_NOT_OK (build_mustache_data(it->second, value));
                object[key] = std::move(value);
            }
            data = object;
            return {};
        }
        default:
            return zuri_project::ProjectStatus::forCondition(
                zuri_project::ProjectCondition::kProjectInvariant, "invalid user argument");
    }
}

// tempo_utils::Status
// zuri_project::validate_user_arguments(
//     std::shared_ptr<TemplateConfig> templateConfig,
//     absl::flat_hash_map<std::string,tempo_config::ConfigNode> &userArguments,
//     bool interactive)
// {
//     for (auto it = templateConfig->parametersBegin(); it != templateConfig->parametersEnd(); ++it) {
//         const auto &paramName = it->first;
//         const auto &paramEntry = it->second;
//         auto entry = userArguments.find(paramName);
//         if (entry == userArguments.cend()) {
//             if (interactive) {
//                 TU_CONSOLE_OUT << "prompt for argument value";
//                 continue;
//             }
//             if (paramEntry->optional) {
//                 continue;
//             }
//             return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
//                 "missing template argument for '{}'", paramName);
//         }
//     }
//     return {};
// }

zuri_project::TemplateProcessor::TemplateProcessor()
    : m_priv(std::make_shared<Priv>())
{
}

zuri_project::TemplateProcessor::TemplateProcessor(std::shared_ptr<TemplateConfig> templateConfig)
    : m_priv(std::make_shared<Priv>())
{
    m_priv->parameterStore.insert(templateConfig->parametersBegin(), templateConfig->parametersEnd());
    for (const auto &entry : m_priv->parameterStore) {
        m_priv->missingArguments.insert(entry.first);
    }
}

zuri_project::TemplateProcessor::TemplateProcessor(
    const absl::flat_hash_map<std::string,std::shared_ptr<const ParameterEntry>> &parameterStore)
    : m_priv(std::make_shared<Priv>())
{
    m_priv->parameterStore.insert(parameterStore.cbegin(), parameterStore.cend());
    for (const auto &entry : m_priv->parameterStore) {
        m_priv->missingArguments.insert(entry.first);
    }
}

tempo_utils::Status
zuri_project::TemplateProcessor::putMetadata(
    const std::string &group,
    const std::string &name,
    const std::string &value)
{
    TU_ASSERT (!group.empty() && !name.empty());
    auto id = absl::StrCat(group, "::", name);
    auto *existing = m_priv->templateContext.get(id);
    if (existing)
        return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
            "metadata '{}' already exists; current value is '{}'",
            id, existing->string_value());
    m_priv->templateContext.set(id, value);
    return {};
}

tempo_utils::Status
zuri_project::TemplateProcessor::putMetadata(
    const std::string &group,
    const std::string &name,
    const tempo_config::ConfigNode &node)
{
    TU_ASSERT (!group.empty() && !name.empty());
    auto id = absl::StrCat(group, "::", name);
    auto *existing = m_priv->templateContext.get(id);
    if (existing)
        return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
            "metadata '{}' already exists; current value is '{}'",
            id, existing->string_value());
    mustache::data data;
    TU_RETURN_IF_NOT_OK (build_mustache_data(node, data));
    m_priv->templateContext.set(id, data);
    return {};
}

tempo_utils::Status
zuri_project::TemplateProcessor::putArgument(const std::string &name, const std::string &value)
{
    if (name.empty())
        return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
            "invalid argument name");
    if (absl::StrContains(name, "::"))
        return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
            "invalid argument name '{}'; user argument name cannot contain '::'", name);
    if (!m_priv->parameterStore.contains(name))
        return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
            "invalid user argument '{}'; template parameter not defined", name);

    auto *existing = m_priv->templateContext.get(name);
    if (existing)
        return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
            "argument '{}' already exists; current value is '{}'", existing->string_value());
    m_priv->templateContext.set(name, value);
    m_priv->missingArguments.erase(name);
    return {};
}

tempo_utils::Status
zuri_project::TemplateProcessor::putArgument(const std::string &name, const tempo_config::ConfigNode &node)
{
    auto *existing = m_priv->templateContext.get(name);
    if (existing)
        return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
            "argument '{}' already exists; current value is '{}'", existing->string_value());
    mustache::data data;
    TU_RETURN_IF_NOT_OK (build_mustache_data(node, data));
    m_priv->templateContext.set(name, data);
    m_priv->missingArguments.erase(name);
    return {};
}

bool
zuri_project::TemplateProcessor::hasParameter(std::string_view name) const
{
    return m_priv->parameterStore.contains(name);
}

std::shared_ptr<const zuri_project::ParameterEntry>
zuri_project::TemplateProcessor::getParameter(std::string_view name) const
{
    auto entry = m_priv->parameterStore.find(name);
    if (entry != m_priv->parameterStore.cend())
        return entry->second;
    return {};
}

absl::flat_hash_set<std::string>::const_iterator
zuri_project::TemplateProcessor::missingBegin() const
{
    return m_priv->missingArguments.cbegin();
}

absl::flat_hash_set<std::string>::const_iterator
zuri_project::TemplateProcessor::missingEnd() const
{
    return m_priv->missingArguments.cend();
}

tempo_utils::Result<std::string>
zuri_project::TemplateProcessor::processTemplate(const std::filesystem::path &templateFile)
{
    tempo_utils::FileReader templateReader(templateFile);
    TU_RETURN_IF_NOT_OK (templateReader.getStatus());
    auto bytes = templateReader.getBytes();
    std::string templateString(bytes->getStringView());
    return processTemplate(templateString);
}

tempo_utils::Result<std::string>
zuri_project::TemplateProcessor::processTemplate(const std::string &templateString)
{
    mustache::mustache mustacheTemplate(templateString);
    if (!mustacheTemplate.is_valid())
        return ProjectStatus::forCondition(ProjectCondition::kProjectInvariant,
            "template contains error; {}", mustacheTemplate.error_message());

    auto templateOutput = mustacheTemplate.render(m_priv->templateContext);
    return templateOutput;
}
