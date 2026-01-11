
#include <lyric_common/common_conversions.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/enum_conversions.h>
#include <tempo_config/parse_config.h>
#include <zuri_packager/packaging_conversions.h>
#include <zuri_tooling/tooling_conversions.h>

tempo_utils::Status
zuri_tooling::ImportEntryParser::parseString(const tempo_config::ConfigValue &value, ImportEntry &importEntry) const
{
    zuri_packager::PackageVersionParser versionParser;
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(importEntry.version, versionParser, value));
    return {};
}

tempo_utils::Status
zuri_tooling::ImportEntryParser::parseVersion(const tempo_config::ConfigMap &map, ImportEntry &importEntry) const
{
    zuri_packager::PackageVersionParser versionParser;
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(importEntry.version, versionParser, map, "version"));
    return {};
}

tempo_utils::Status
zuri_tooling::ImportEntryParser::parsePath(const tempo_config::ConfigMap &map, ImportEntry &importEntry) const
{
    tempo_config::PathParser pathParser;
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(importEntry.path, pathParser, map, "path"));
    return {};
}

tempo_utils::Status
zuri_tooling::ImportEntryParser::convertValue(const tempo_config::ConfigNode &node, ImportEntry &importEntry) const
{
    switch (node.getNodeType()) {
        case tempo_config::ConfigNodeType::kValue:
            return parseString(node.toValue(), importEntry);

        case tempo_config::ConfigNodeType::kMap: {
            auto importConfig = node.toMap();

            // parse type
            tempo_config::EnumTParser<ImportEntryType> importEntryTypeParser({
                {"Version", ImportEntryType::Version},
                {"Path", ImportEntryType::Path},
            });
            TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
                importEntry.type, importEntryTypeParser, importConfig, "type"));

            switch (importEntry.type) {
                case ImportEntryType::Version:
                    return parseVersion(importConfig, importEntry);
                case ImportEntryType::Path:
                    return parsePath(importConfig, importEntry);
                default:
                    return tempo_config::ConfigStatus::forCondition(
                        tempo_config::ConfigCondition::kParseError, "invalid import entry type");
            }
        }

        default:
            return tempo_config::ConfigStatus::forCondition(
                tempo_config::ConfigCondition::kParseError, "import entry config must be a string or map");
    }
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
            return tempo_config::ConfigStatus::forCondition(
                tempo_config::ConfigCondition::kParseError, "invalid target entry type");
    }
}

tempo_utils::Status
zuri_tooling::RepositoryEntryParser::parseDirectory(const tempo_config::ConfigMap &map, RepositoryEntry &repositoryEntry) const
{
    tempo_config::PathParser directoryPathParser;
    RepositoryEntry::Directory directory;

    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(directory.directoryPath, directoryPathParser,
        map, "directoryPath"));
    repositoryEntry.repository = std::move(directory);

    return {};
}

tempo_utils::Status
zuri_tooling::RepositoryEntryParser::parseHttps(const tempo_config::ConfigMap &map, RepositoryEntry &repositoryEntry) const
{
    tempo_config::UrlParser httpsLocationParser;
    RepositoryEntry::Https https;

    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(https.httpsLocation, httpsLocationParser,
        map, "httpsLocation"));
    repositoryEntry.repository = std::move(https);

    return {};
}

tempo_utils::Status
zuri_tooling::RepositoryEntryParser::convertValue(const tempo_config::ConfigNode &node, RepositoryEntry &repositoryEntry) const
{
    if (node.getNodeType() != tempo_config::ConfigNodeType::kMap)
        return tempo_config::ConfigStatus::forCondition(
            tempo_config::ConfigCondition::kWrongType, "repository entry config must be a map");
    auto repositoryConfig = node.toMap();

    // parse type
    tempo_config::EnumTParser<RepositoryEntryType> repositoryEntryTypeParser({
        {"Directory", RepositoryEntryType::Directory},
        {"Https", RepositoryEntryType::Https},
    });
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
        repositoryEntry.type, repositoryEntryTypeParser, repositoryConfig, "type"));

    // parse priority
    tempo_config::IntegerParser priorityParser(100);
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
        repositoryEntry.priority, priorityParser, repositoryConfig, "priority"));

    // parse type specific members
    switch (repositoryEntry.type) {
        case RepositoryEntryType::Directory:
            return parseDirectory(repositoryConfig, repositoryEntry);
        case RepositoryEntryType::Https:
            return parseHttps(repositoryConfig, repositoryEntry);
        default:
            return tempo_config::ConfigStatus::forCondition(
                tempo_config::ConfigCondition::kParseError, "invalid repository entry type");
    }
}