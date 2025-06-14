
#include <tempo_config/config_result.h>
#include <zuri_packager/packaging_conversions.h>

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
