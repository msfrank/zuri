
#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/enum_conversions.h>
#include <tempo_config/parse_config.h>
#include <zuri_build/import_store.h>

ImportStore::ImportStore(const tempo_config::ConfigMap &importsConfig)
    : m_importsConfig(importsConfig)
{
}

class ImportEntryParser : public tempo_config::AbstractConverter<ImportEntry> {
public:
    static tempo_utils::Status parseTarget(const tempo_config::ConfigMap &map, ImportEntry &importEntry) {
        tempo_config::StringParser targetNameParser;
        TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
            importEntry.targetName, targetNameParser, map, "targetName"));
        return {};
    }

    static tempo_utils::Status parseRequirement(const tempo_config::ConfigMap &map, ImportEntry &importEntry) {
        tempo_config::StringParser requirementSpecParser;
        TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
            importEntry.requirementSpec, requirementSpecParser, map, "requirementSpec"));
        return {};
    }

    static tempo_utils::Status parsePackage(const tempo_config::ConfigMap &map, ImportEntry &importEntry) {
        tempo_config::UrlParser packageUrlParser;
        TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
            importEntry.packageUrl, packageUrlParser, map, "packageUrl"));
        return {};
    }

    tempo_utils::Status convertValue(const tempo_config::ConfigNode &node, ImportEntry &importEntry) const override {
        if (node.getNodeType() != tempo_config::ConfigNodeType::kMap)
            return tempo_config::ConfigStatus::forCondition(
                tempo_config::ConfigCondition::kWrongType, "import entry config must be a map");
        auto importConfig = node.toMap();

        tempo_config::EnumTParser<ImportEntryType> importEntryTypeParser({
            {"Target", ImportEntryType::Target},
            {"Requirement", ImportEntryType::Requirement},
            {"Package", ImportEntryType::Package},
        });
        TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
            importEntry.type, importEntryTypeParser, importConfig, "type"));
        switch (importEntry.type) {
            case ImportEntryType::Target:
                return parseTarget(importConfig, importEntry);
            case ImportEntryType::Requirement:
                return parseRequirement(importConfig, importEntry);
            case ImportEntryType::Package:
                return parsePackage(importConfig, importEntry);
            default:
                return {};
        }
    }
};

tempo_utils::Status
ImportStore::configure()
{
    tempo_config::StringParser importNameParser;
    ImportEntryParser importEntryParser;
    tempo_config::MapKVParser importEntriesParser(&importNameParser, &importEntryParser);

    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
        m_importEntries, importEntriesParser, m_importsConfig));

    return {};
}

bool
ImportStore::hasImport(const std::string &importName) const
{
    return m_importEntries.contains(importName);
}

const ImportEntry&
ImportStore::getImport(const std::string &importName) const
{
    return m_importEntries.at(importName);
}

absl::flat_hash_map<std::string,ImportEntry>::const_iterator
ImportStore::importsBegin() const
{
    return m_importEntries.cbegin();
}

absl::flat_hash_map<std::string,ImportEntry>::const_iterator
ImportStore::importsEnd() const
{
    return m_importEntries.cend();
}

int
ImportStore::numImports() const
{
    return m_importEntries.size();
}
