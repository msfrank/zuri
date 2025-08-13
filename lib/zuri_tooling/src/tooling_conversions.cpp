
#include <lyric_common/common_conversions.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/enum_conversions.h>
#include <tempo_config/parse_config.h>
#include <zuri_packager/packaging_conversions.h>
#include <zuri_tooling/tooling_conversions.h>

tempo_utils::Status
zuri_tooling::ImportEntryParser::parseTarget(const tempo_config::ConfigMap &map, ImportEntry &importEntry)
{
    tempo_config::StringParser targetNameParser;
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
        importEntry.targetName, targetNameParser, map, "targetName"));
    return {};
}

tempo_utils::Status
zuri_tooling::ImportEntryParser::parseRequirement(const tempo_config::ConfigMap &map, ImportEntry &importEntry)
{
    zuri_packager::PackageSpecifierParser requirementSpecifierParser;
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
        importEntry.requirementSpecifier, requirementSpecifierParser, map, "requirementSpecifier"));
    return {};
}

tempo_utils::Status
zuri_tooling::ImportEntryParser::parsePackage(const tempo_config::ConfigMap &map, ImportEntry &importEntry)
{
    tempo_config::UrlParser packageUrlParser;
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
        importEntry.packageUrl, packageUrlParser, map, "packageUrl"));
    return {};
}

tempo_utils::Status
zuri_tooling::ImportEntryParser::convertValue(const tempo_config::ConfigNode &node, ImportEntry &importEntry) const
{
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

tempo_utils::Status
zuri_tooling::TargetEntryParser::parseProgram(const tempo_config::ConfigMap &map, TargetEntry &targetEntry) const
{
    lyric_common::ModuleLocationParser moduleLocationParser;
    tempo_config::SeqTParser libraryModulesParser(&moduleLocationParser, {});
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
        targetEntry.modules, libraryModulesParser, map, "programModules"));
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
        targetEntry.main, moduleLocationParser, map, "programMain"));
    return {};
}

tempo_utils::Status
zuri_tooling::TargetEntryParser::parseLibrary(const tempo_config::ConfigMap &map, TargetEntry &targetEntry) const
{
    lyric_common::ModuleLocationParser moduleLocationParser;
    tempo_config::SeqTParser libraryModulesParser(&moduleLocationParser);
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
        targetEntry.modules, libraryModulesParser, map, "libraryModules"));
    return {};
}

tempo_utils::Status
zuri_tooling::TargetEntryParser::parseArchive(const tempo_config::ConfigMap &map, TargetEntry &targetEntry) const
{
    lyric_common::ModuleLocationParser moduleLocationParser;
    tempo_config::SeqTParser archiveModulesParser(&moduleLocationParser);
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
        targetEntry.modules, archiveModulesParser, map, "archiveModules"));
    return {};
}

tempo_utils::Status
zuri_tooling::TargetEntryParser::convertValue(const tempo_config::ConfigNode &node, TargetEntry &targetEntry) const
{
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