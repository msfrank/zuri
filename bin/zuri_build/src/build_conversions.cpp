
#include <tempo_config/base_conversions.h>
#include <tempo_config/enum_conversions.h>
#include <tempo_config/parse_config.h>
#include <zuri_build/build_conversions.h>


tempo_utils::Status
zuri_build::ExistingArtifactParser::parseFile(const tempo_config::ConfigMap &map, ExistingArtifact &existingArtifact) const
{
    tempo_config::UrlPathParser filePathParser;
    ExistingArtifact::FileConfig fileConfig;
    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(fileConfig.filePath, filePathParser, map, "filePath"));
    existingArtifact.type = ExistingArtifact::Type::File;
    existingArtifact.config = std::move(fileConfig);
    return {};
}

tempo_utils::Status
zuri_build::ExistingArtifactParser::parseExternalFile(const tempo_config::ConfigMap &map, ExistingArtifact &existingArtifact) const
{
    tempo_config::PathParser externalFilePathParser;
    ExistingArtifact::ExternalFileConfig externalFileConfig;
    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(externalFileConfig.externalFilePath, externalFilePathParser,
        map, "externalFilePath"));
    existingArtifact.type = ExistingArtifact::Type::ExternalFile;
    existingArtifact.config = std::move(externalFileConfig);
    return {};
}

tempo_utils::Status
zuri_build::ExistingArtifactParser::parseUrl(const tempo_config::ConfigMap &map, ExistingArtifact &existingArtifact) const
{
    tempo_config::UrlParser urlParser;
    ExistingArtifact::UrlConfig urlConfig;
    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(urlConfig.url, urlParser, map, "url"));
    existingArtifact.type = ExistingArtifact::Type::Url;
    existingArtifact.config = std::move(urlConfig);
    return {};
}

tempo_utils::Status
zuri_build::ExistingArtifactParser::convertValue(const tempo_config::ConfigNode &node, ExistingArtifact &existingArtifact) const
{
    tempo_config::ConfigMap existingMap;
    switch (node.getNodeType()) {
        case tempo_config::ConfigNodeType::kValue: {
            tempo_config::UrlPathParser filePathParser;
            ExistingArtifact::FileConfig fileConfig;
            TU_RETURN_IF_NOT_OK(tempo_config::parse_config(fileConfig.filePath, filePathParser, node));
            existingArtifact.type = ExistingArtifact::Type::File;
            existingArtifact.config = std::move(fileConfig);
            return {};
        }
        case tempo_config::ConfigNodeType::kMap:
            existingMap = node.toMap();
            break;
        default:
            return tempo_config::ConfigStatus::forCondition(
                tempo_config::ConfigCondition::kWrongType, "existing artifact config must be a value or map");
    }

    // parse type
    tempo_config::EnumTParser<ExistingArtifact::Type> existingArtifactTypeParser({
        {"File", ExistingArtifact::Type::File},
        {"ExternalFile", ExistingArtifact::Type::ExternalFile},
        {"Url", ExistingArtifact::Type::Url},
    });
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
        existingArtifact.type, existingArtifactTypeParser, existingMap, "type"));

    // parse type specific members
    switch (existingArtifact.type) {
        case ExistingArtifact::Type::File:
            return parseFile(existingMap, existingArtifact);
        case ExistingArtifact::Type::ExternalFile:
            return parseExternalFile(existingMap, existingArtifact);
        case ExistingArtifact::Type::Url:
            return parseUrl(existingMap, existingArtifact);
        default:
            return tempo_config::ConfigStatus::forCondition(
                tempo_config::ConfigCondition::kParseError, "invalid existing artifact type");
    }
}
