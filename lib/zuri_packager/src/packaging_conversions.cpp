
#include <tempo_config/config_result.h>
#include <zuri_packager/packaging_conversions.h>

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
    if (node.isNil()) {
        if (m_default.isEmpty())
            return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kMissingValue,
                "missing required package id value");
        id = m_default.getValue();
        return {};
    }
    if (node.getNodeType() != tempo_config::ConfigNodeType::kValue)
        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kWrongType,
            "expected Value node but found {}", config_node_type_to_string(node.getNodeType()));

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

zuri_packager::PackageVersionParser::PackageVersionParser()
{
}

zuri_packager::PackageVersionParser::PackageVersionParser(const PackageVersion &versionDefault)
    : m_default(versionDefault)
{
}

tempo_utils::Status
zuri_packager::PackageVersionParser::convertValue(
    const tempo_config::ConfigNode &node,
    PackageVersion &version) const
{
    if (node.isNil()) {
        if (m_default.isEmpty())
            return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kMissingValue,
                "missing required package version value");
        version = m_default.getValue();
        return {};
    }
    if (node.getNodeType() != tempo_config::ConfigNodeType::kValue)
        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kWrongType,
            "expected Value node but found {}", config_node_type_to_string(node.getNodeType()));

    auto value = node.toValue().getValue();
    auto v = PackageVersion::fromString(value);
    if (!v.isValid())
        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kParseError,
            "invalid package version");
    version = v;

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
    if (node.isNil()) {
        if (m_default.isEmpty())
            return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kMissingValue,
                "missing required package specifier value");
        specifier = m_default.getValue();
        return {};
    }
    if (node.getNodeType() != tempo_config::ConfigNodeType::kValue)
        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kWrongType,
            "expected Value node but found {}", config_node_type_to_string(node.getNodeType()));

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
    if (node.isNil()) {
        if (m_default.isEmpty())
            return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kMissingValue,
                "missing required requirements map");
        requirementsMap = m_default.getValue();
        return {};
    }

    if (node.getNodeType() != tempo_config::ConfigNodeType::kMap)
        return tempo_config::ConfigStatus::forCondition(
            tempo_config::ConfigCondition::kWrongType, "requirements must be a map");

    auto map = node.toMap();
    absl::flat_hash_map<PackageId,PackageVersion> requirements;
    for (auto it = map.mapBegin(); it != map.mapEnd(); ++it) {
        if (it->second.getNodeType() != tempo_config::ConfigNodeType::kValue)
            return tempo_config::ConfigStatus::forCondition(
                tempo_config::ConfigCondition::kParseError, "requirement entry must be a value");
        auto id = PackageId::fromString(it->first);
        auto value = it->second.toValue();
        auto version = PackageVersion::fromString(value.getValue());
        requirements[id] = version;
    }
    requirementsMap = RequirementsMap(requirements);

    return {};
}

zuri_packager::LibrariesNeededParser::LibrariesNeededParser()
{
}

zuri_packager::LibrariesNeededParser::LibrariesNeededParser(
    const LibrariesNeeded &librariesNeededDefault)
    : m_default(librariesNeededDefault)
{
}

tempo_utils::Status
zuri_packager::LibrariesNeededParser::convertValue(
    const tempo_config::ConfigNode &node,
    LibrariesNeeded &librariesNeeded) const
{
    if (node.isNil()) {
        if (m_default.isEmpty())
            return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kMissingValue,
                "missing required librariesNeeded map");
        librariesNeeded = m_default.getValue();
        return {};
    }

    if (node.getNodeType() != tempo_config::ConfigNodeType::kMap)
        return tempo_config::ConfigStatus::forCondition(
            tempo_config::ConfigCondition::kWrongType, "librariesNeeded must be a map");

    LibrariesNeeded needed;

    auto map = node.toMap();
    for (auto mapIt = map.mapBegin(); mapIt != map.mapEnd(); ++mapIt) {
        auto &librarySource = mapIt->first;
        if (librarySource.empty())
            return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kParseError,
                "librariesNeeded contains invalid entry; key is empty");

        if (mapIt->second.getNodeType() != tempo_config::ConfigNodeType::kSeq)
            return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kParseError,
                "librariesNeeded value for {} must be a seq", librarySource);
        auto seq = mapIt->second.toSeq();

        absl::flat_hash_set<std::string> libraryNames;
        for (auto seqIt = seq.seqBegin(); seqIt != seq.seqEnd(); ++seqIt) {
            if (seqIt->getNodeType() != tempo_config::ConfigNodeType::kValue)
                return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kParseError,
                    "librariesNeeded value for {} contains member with invalid type", librarySource);
            libraryNames.insert(seqIt->toValue().getValue());
        }

        if (librarySource == "$SYSTEM$") {
            needed.addSystemLibraries(libraryNames);
        } else if (librarySource == "$DISTRIBUTION$") {
            needed.addDistributionLibraries(libraryNames);
        } else {
            auto packageId = PackageId::fromString(librarySource);
            if (!packageId.isValid())
                return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kParseError,
                    "librariesNeeded key '{}' is not a invalid packageId", librarySource);
            needed.addPackageLibraries(packageId, libraryNames);
        }
    }

    librariesNeeded = needed;

    return {};
}

zuri_packager::LibrariesProvidedParser::LibrariesProvidedParser()
{
}

zuri_packager::LibrariesProvidedParser::LibrariesProvidedParser(
    const LibrariesProvided &librariesProvidedDefault)
    : m_default(librariesProvidedDefault)
{
}

tempo_utils::Status
zuri_packager::LibrariesProvidedParser::convertValue(
    const tempo_config::ConfigNode &node,
    LibrariesProvided &librariesProvided) const
{
    if (node.isNil()) {
        if (m_default.isEmpty())
            return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kMissingValue,
                "missing required librariesProvided seq");
        librariesProvided = m_default.getValue();
        return {};
    }

    if (node.getNodeType() != tempo_config::ConfigNodeType::kSeq)
        return tempo_config::ConfigStatus::forCondition(
            tempo_config::ConfigCondition::kWrongType, "librariesProvided must be a seq");
    auto seq = node.toSeq();

    LibrariesProvided provided;

    for (auto it = seq.seqBegin(); it != seq.seqEnd(); ++it) {
        if (it->getNodeType() != tempo_config::ConfigNodeType::kValue)
            return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kParseError,
                "librariesProvided contains member with invalid type");
        provided.addLibrary(it->toValue().getValue());
    }

    librariesProvided = provided;

    return {};
}
