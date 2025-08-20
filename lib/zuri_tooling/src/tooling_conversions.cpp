
#include <lyric_common/common_conversions.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/enum_conversions.h>
#include <tempo_config/parse_config.h>
#include <zuri_packager/packaging_conversions.h>
#include <zuri_tooling/tooling_conversions.h>

tempo_utils::Status
zuri_tooling::ImportEntryParser::convertValue(const tempo_config::ConfigNode &node, ImportEntry &importEntry) const
{
    zuri_packager::PackageVersionParser versionParser;
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(importEntry.version, versionParser, node));
    return {};
}

tempo_utils::Status
zuri_tooling::TargetEntryParser::parseProgram(const tempo_config::ConfigMap &map, TargetEntry &targetEntry) const
{
    zuri_packager::PackageSpecifierParser specifierParser;
    lyric_common::ModuleLocationParser moduleLocationParser;
    tempo_config::SeqTParser programModulesParser(&moduleLocationParser, {});
    TargetEntry::Program program;

    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
        program.specifier, specifierParser, map, "specifier"));
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
        program.modules, programModulesParser, map, "programModules"));
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
        program.main, moduleLocationParser, map, "programMain"));
    targetEntry.target = std::move(program);

    return {};
}

tempo_utils::Status
zuri_tooling::TargetEntryParser::parseLibrary(const tempo_config::ConfigMap &map, TargetEntry &targetEntry) const
{
    zuri_packager::PackageSpecifierParser specifierParser;
    lyric_common::ModuleLocationParser moduleLocationParser;
    tempo_config::SeqTParser libraryModulesParser(&moduleLocationParser);
    TargetEntry::Library library;

    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
        library.specifier, specifierParser, map, "specifier"));
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
        library.modules, libraryModulesParser, map, "libraryModules"));
    targetEntry.target = std::move(library);

    return {};
}

tempo_utils::Status
zuri_tooling::TargetEntryParser::parsePackage(const tempo_config::ConfigMap &map, TargetEntry &targetEntry) const
{
    tempo_config::UrlParser urlParser;
    TargetEntry::Package package;

    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
        package.url, urlParser, map, "packageUrl"));
    targetEntry.target = std::move(package);

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
        {"Library", TargetEntryType::Library},
        {"Program", TargetEntryType::Program},
        {"Package", TargetEntryType::Package},
    });
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
        targetEntry.type, targetEntryTypeParser, targetConfig, "type"));

    // parse depends
    tempo_config::StringParser dependencyParser;
    tempo_config::SeqTParser dependsParser(&dependencyParser, {});
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
        targetEntry.depends, dependsParser, targetConfig, "depends"));

    // parse type specific members
    switch (targetEntry.type) {
        case TargetEntryType::Program:
            return parseProgram(targetConfig, targetEntry);
        case TargetEntryType::Library:
            return parseLibrary(targetConfig, targetEntry);
        case TargetEntryType::Package:
            return parsePackage(targetConfig, targetEntry);
        default:
            return {};
    }
}

tempo_utils::Status
zuri_tooling::PackageCacheEntryParser::convertValue(
    const tempo_config::ConfigNode &node,
    PackageCacheEntry &packageCacheEntry) const
{
    if (node.getNodeType() != tempo_config::ConfigNodeType::kMap)
        return tempo_config::ConfigStatus::forCondition(
            tempo_config::ConfigCondition::kWrongType, "target entry config must be a map");
    auto packageCacheConfig = node.toMap();

    tempo_config::BooleanParser writeableParser(false);
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
        packageCacheEntry.writeable, writeableParser, packageCacheConfig, "writeable"));
    return {};
}
