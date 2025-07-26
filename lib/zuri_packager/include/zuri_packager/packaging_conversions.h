#ifndef ZURI_PACKAGER_PACKAGING_CONVERSIONS_H
#define ZURI_PACKAGER_PACKAGING_CONVERSIONS_H

#include <tempo_config/abstract_converter.h>
#include <tempo_utils/status.h>

#include "package_requirement.h"
#include "package_specifier.h"

namespace zuri_packager {

    class PackageIdParser : public tempo_config::AbstractConverter<PackageId> {
    public:
        PackageIdParser();
        explicit PackageIdParser(const PackageId &idDefault);
        tempo_utils::Status convertValue(
            const tempo_config::ConfigNode &node,
            PackageId &id) const override;

    private:
        Option<PackageId> m_default;
    };

    class PackageVersionParser : public tempo_config::AbstractConverter<PackageVersion> {
    public:
        PackageVersionParser();
        explicit PackageVersionParser(const PackageVersion &versionDefault);
        tempo_utils::Status convertValue(
            const tempo_config::ConfigNode &node,
            PackageVersion &version) const override;

    private:
        Option<PackageVersion> m_default;
    };

    class PackageSpecifierParser : public tempo_config::AbstractConverter<PackageSpecifier> {
    public:
        PackageSpecifierParser();
        explicit PackageSpecifierParser(const PackageSpecifier &specifierDefault);
        tempo_utils::Status convertValue(
            const tempo_config::ConfigNode &node,
            PackageSpecifier &specifier) const override;

    private:
        Option<PackageSpecifier> m_default;
    };

    class RequirementsMapParser : public tempo_config::AbstractConverter<RequirementsMap> {
    public:
        RequirementsMapParser();
        explicit RequirementsMapParser(const RequirementsMap &requirementsMapDefault);
        tempo_utils::Status convertValue(
            const tempo_config::ConfigNode &node,
            RequirementsMap &requirementsMap) const override;

    private:
        Option<RequirementsMap> m_default;
    };

    // class RequirementsListParser : public tempo_config::AbstractConverter<RequirementsList> {
    // public:
    //     RequirementsListParser();
    //     explicit RequirementsListParser(const RequirementsList &requirementsDefault);
    //     tempo_utils::Status convertValue(
    //         const tempo_config::ConfigNode &node,
    //         RequirementsList &requirements) const override;
    //
    // private:
    //     Option<RequirementsList> m_default;
    // };
}

#endif // ZURI_PACKAGER_PACKAGING_CONVERSIONS_H
