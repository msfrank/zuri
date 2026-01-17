#ifndef ZURI_PROJECT_TEMPLATE_H
#define ZURI_PROJECT_TEMPLATE_H

#include <filesystem>

#include <tempo_config/config_types.h>
#include <tempo_utils/result.h>

namespace zuri_project {

    /**
     * The file name of the template config file within a template.
     */
    constexpr const char * const kTemplateConfigName = "template.config";

    /**
     * The file name of the target.config template file within a template.
     */
    constexpr const char * const kTargetConfigTemplateName = "target.config.mustache";

    /**
     * A Template is a directory structure containing the files and directories needed to generate
     * a target from a template.
     */
    class Template {
    public:
        Template();
        Template(const Template &other);

        bool isValid() const;

        std::filesystem::path getTemplateConfigFile() const;
        std::filesystem::path getTemplateDirectory() const;
        std::filesystem::path getTargetConfigTemplateFile() const;

        static tempo_utils::Result<Template> open(const std::filesystem::path &templateDirectoryOrConfigFile);

    private:
        struct Priv {
            std::filesystem::path templateConfigFile;
            std::filesystem::path templateDirectory;
            std::filesystem::path targetConfigTemplateFile;
        };
        std::shared_ptr<Priv> m_priv;

        Template(
            const std::filesystem::path &templateConfigFile,
            const std::filesystem::path &templateDirectory,
            const std::filesystem::path &targetConfigTemplateFile);
    };
}

#endif // ZURI_PROJECT_TEMPLATE_H