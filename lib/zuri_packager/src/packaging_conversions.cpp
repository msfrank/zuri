
#include <tempo_config/config_result.h>
#include <zuri_packager/packaging_conversions.h>

#include "zuri_packager/requirement_parser.h"

zuri_packager::PackageIdParser::PackageIdParser()
{
}

zuri_packager::PackageIdParser::PackageIdParser(const PackageId &idDefault)
    : m_default(idDefault)
{
}

tempo_utils::Status
zuri_packager::PackageIdParser::convertValue(
    const tempo_config::ConfigNode &node,
    PackageId &id) const
{
    if (node.isNil() && !m_default.isEmpty()) {
        id = m_default.getValue();
        return {};
    }
    if (node.getNodeType() != tempo_config::ConfigNodeType::kValue)
        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kWrongType);

    auto value = node.toValue().getValue();
    auto authority = tempo_utils::UrlAuthority::fromString(value);
    auto name = authority.getUsername();
    if (name.empty())
        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kParseError,
            "missing package id name");
    auto domain = authority.getHost();
    if (domain.empty())
        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kParseError,
            "missing package id domain");
    id = PackageId(name, domain);

    return {};
}

zuri_packager::PackageSpecifierParser::PackageSpecifierParser()
{
}

zuri_packager::PackageSpecifierParser::PackageSpecifierParser(const PackageSpecifier &specifierDefault)
    : m_default(specifierDefault)
{
}

tempo_utils::Status
zuri_packager::PackageSpecifierParser::convertValue(
    const tempo_config::ConfigNode &node,
    PackageSpecifier &specifier) const
{
    if (node.isNil() && !m_default.isEmpty()) {
        specifier = m_default.getValue();
        return {};
    }
    if (node.getNodeType() != tempo_config::ConfigNodeType::kValue)
        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kWrongType);

    auto value = node.toValue().getValue();
    auto authority = tempo_utils::UrlAuthority::fromString(value);
    specifier = PackageSpecifier::fromAuthority(authority);

    return {};
}

zuri_packager::RequirementsMapParser::RequirementsMapParser()
{
}

zuri_packager::RequirementsMapParser::RequirementsMapParser(
    const RequirementsMap &requirementsMapDefault)
    : m_default(requirementsMapDefault)
{
}

tempo_utils::Status
zuri_packager::RequirementsMapParser::convertValue(
    const tempo_config::ConfigNode &node,
    RequirementsMap &requirementsMap) const
{
    if (node.getNodeType() == tempo_config::ConfigNodeType::kMap) {
        auto map = node.toMap();
        absl::flat_hash_map<PackageId,PackageVersion> requirements;
        for (auto it = map.mapBegin(); it != map.mapEnd(); ++it) {
            if (it->second.getNodeType() != tempo_config::ConfigNodeType::kValue)
                return tempo_config::ConfigStatus::forCondition(
                    tempo_config::ConfigCondition::kWrongType, "requirement entry must be a value");
            auto id = PackageId::fromString(it->first);
            auto value = it->second.toValue();
            auto version = PackageVersion::fromString(value.getValue());
            requirements[id] = version;
        }
        requirementsMap = RequirementsMap(requirements);
        return {};
    }

    return tempo_config::ConfigStatus::forCondition(
        tempo_config::ConfigCondition::kWrongType, "requirements must be a map");
}
