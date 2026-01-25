#ifndef ZURI_PROJECT_TEMPLATE_PROCESSOR_H
#define ZURI_PROJECT_TEMPLATE_PROCESSOR_H

#include <filesystem>

#include <tempo_config/config_types.h>
#include <tempo_utils/result.h>

#include "template_config.h"

namespace zuri_project {

    class TemplateProcessor {
    public:
        TemplateProcessor();
        explicit TemplateProcessor(std::shared_ptr<TemplateConfig> templateConfig);
        explicit TemplateProcessor(
            const absl::flat_hash_map<std::string,std::shared_ptr<const ParameterEntry>> &parameterStore);

        tempo_utils::Status putMetadata(
            const std::string &group,
            const std::string &name,
            const std::string &value);
        tempo_utils::Status putMetadata(
            const std::string &group,
            const std::string &name,
            const tempo_config::ConfigNode &node);

        tempo_utils::Status putArgument(const std::string &name, const std::string &value);
        tempo_utils::Status putArgument(const std::string &name, const tempo_config::ConfigNode &node);

        bool hasParameter(std::string_view name) const;
        std::shared_ptr<const ParameterEntry> getParameter(std::string_view name) const;
        absl::flat_hash_set<std::string>::const_iterator missingBegin() const;
        absl::flat_hash_set<std::string>::const_iterator missingEnd() const;

        tempo_utils::Result<std::string> processTemplate(const std::filesystem::path &templateFile);
        tempo_utils::Result<std::string> processTemplate(const std::string &templateString);

    private:
        struct Priv;
        std::shared_ptr<Priv> m_priv;
    };
}

#endif // ZURI_PROJECT_TEMPLATE_PROCESSOR_H