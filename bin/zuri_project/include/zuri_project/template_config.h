#ifndef ZURI_PROJECT_TEMPLATE_CONFIG_H
#define ZURI_PROJECT_TEMPLATE_CONFIG_H

#include <tempo_config/config_types.h>
#include <tempo_utils/result.h>

#include "template.h"
#include "zuri_packager/package_types.h"

namespace zuri_project {

    struct ParameterEntry {
        tempo_config::ConfigNode dfl;
        bool optional;
    };

    /**
     *
     */
    class TemplateConfig {

    public:
        static tempo_utils::Result<std::shared_ptr<TemplateConfig>> load(const Template &tmpl);

        Template getTemplate() const;
        std::string getName() const;
        std::filesystem::path getContentRoot() const;

        bool hasParameter(std::string_view name) const;
        std::shared_ptr<const ParameterEntry> getParameter(std::string_view name) const;
        absl::flat_hash_map<std::string,std::shared_ptr<const ParameterEntry>>::const_iterator parametersBegin() const;
        absl::flat_hash_map<std::string,std::shared_ptr<const ParameterEntry>>::const_iterator parametersEnd() const;
        int numParameters() const;

    private:
        Template m_template;
        tempo_config::ConfigMap m_configMap;

        std::string m_name;
        std::filesystem::path m_contentRoot;
        absl::flat_hash_map<std::string,std::shared_ptr<const ParameterEntry>> m_parameterStore;
        absl::flat_hash_map<zuri_packager::PackageId,zuri_packager::PackageVersion> m_requirements;

        TemplateConfig(
            const Template &tmpl,
            const tempo_config::ConfigMap &configMap);

        tempo_utils::Status configure();
    };
}

#endif // ZURI_PROJECT_TEMPLATE_CONFIG_H