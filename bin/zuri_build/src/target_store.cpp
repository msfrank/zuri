
#include <lyric_common/common_conversions.h>
#include <tempo_config/abstract_converter.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/config_result.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/enum_conversions.h>
#include <tempo_config/parse_config.h>
#include <zuri_build/target_store.h>

#include "zuri_packager/packaging_conversions.h"
#include "zuri_packager/zuri_manifest.h"

TargetStore::TargetStore(const tempo_config::ConfigMap &targetsConfig)
    : m_targetsConfig(targetsConfig)
{
}

class TargetEntryParser : public tempo_config::AbstractConverter<TargetEntry> {
public:
    tempo_utils::Status parseProgram(const tempo_config::ConfigMap &map, TargetEntry &targetEntry) const {
        lyric_common::ModuleLocationParser moduleLocationParser;
        tempo_config::SeqTParser libraryModulesParser(&moduleLocationParser, {});
        TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
            targetEntry.modules, libraryModulesParser, map, "programModules"));
        TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
            targetEntry.main, moduleLocationParser, map, "programMain"));
        return {};
    }

    tempo_utils::Status parseLibrary(const tempo_config::ConfigMap &map, TargetEntry &targetEntry) const {
        lyric_common::ModuleLocationParser moduleLocationParser;
        tempo_config::SeqTParser libraryModulesParser(&moduleLocationParser);
        TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
            targetEntry.modules, libraryModulesParser, map, "libraryModules"));
        return {};
    }

    tempo_utils::Status parseArchive(const tempo_config::ConfigMap &map, TargetEntry &targetEntry) const {
        lyric_common::ModuleLocationParser moduleLocationParser;
        tempo_config::SeqTParser archiveModulesParser(&moduleLocationParser);
        TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
            targetEntry.modules, archiveModulesParser, map, "archiveModules"));
        return {};
    }

    tempo_utils::Status convertValue(const tempo_config::ConfigNode &node, TargetEntry &targetEntry) const override {
        if (node.getNodeType() != tempo_config::ConfigNodeType::kMap)
            return tempo_config::ConfigStatus::forCondition(
                tempo_config::ConfigCondition::kWrongType, "target entry config must be a map");
        auto targetConfig = node.toMap();

        // parse type
        tempo_config::EnumTParser<TargetEntryType> targetEntryTypeParser({
            {"Program", TargetEntryType::Program},
            {"Library", TargetEntryType::Library},
            {"Archive", TargetEntryType::Archive},
        });
        TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
            targetEntry.type, targetEntryTypeParser, targetConfig, "type"));

        // parse specifier
        zuri_packager::PackageSpecifierParser specifierParser;
        TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
            targetEntry.specifier, specifierParser, targetConfig, "specifier"));

        // parse depends
        tempo_config::StringParser dependencyParser;
        tempo_config::SeqTParser dependsParser(&dependencyParser, {});
        TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
            targetEntry.depends, dependsParser, targetConfig, "depends"));

        // parse imports
        tempo_config::StringParser importParser;
        tempo_config::SeqTParser importsParser(&importParser, {});
        TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
            targetEntry.imports, importsParser, targetConfig, "imports"));

        // parse type specific members
        switch (targetEntry.type) {
            case TargetEntryType::Program:
                return parseProgram(targetConfig, targetEntry);
            case TargetEntryType::Library:
                return parseLibrary(targetConfig, targetEntry);
            case TargetEntryType::Archive:
                return parseArchive(targetConfig, targetEntry);
            default:
                return {};
        }
    }
};

tempo_utils::Status
TargetStore::configure()
{
    tempo_config::StringParser targetNameParser;
    TargetEntryParser targetEntryParser;
    tempo_config::MapKVParser<std::string,TargetEntry> targetEntriesParser(&targetNameParser, &targetEntryParser);

    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
        m_targetEntries, targetEntriesParser, m_targetsConfig));

    return {};
}

bool
TargetStore::hasTarget(const std::string &targetName) const
{
    return m_targetEntries.contains(targetName);
}

const TargetEntry&
TargetStore::getTarget(const std::string &targetName) const
{
    return m_targetEntries.at(targetName);
}

absl::flat_hash_map<std::string,TargetEntry>::const_iterator
TargetStore::targetsBegin() const
{
    return m_targetEntries.cbegin();
}

absl::flat_hash_map<std::string,TargetEntry>::const_iterator
TargetStore::targetsEnd() const
{
    return m_targetEntries.cend();
}

int
TargetStore::numTargets() const
{
    return m_targetEntries.size();
}
