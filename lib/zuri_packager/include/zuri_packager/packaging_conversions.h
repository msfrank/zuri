#ifndef ZURI_PACKAGER_PACKAGING_CONVERSIONS_H
#define ZURI_PACKAGER_PACKAGING_CONVERSIONS_H

#include <tempo_config/abstract_converter.h>
#include <tempo_utils/status.h>

#include "package_specifier.h"

namespace zuri_packager {

    class PackageSpecifierParser : public tempo_config::AbstractConverter<PackageSpecifier> {
    public:
        PackageSpecifierParser();
        PackageSpecifierParser(const PackageSpecifier &specifierDefault);
        tempo_utils::Status convertValue(
            const tempo_config::ConfigNode &node,
            PackageSpecifier &specifier) const override;

    private:
        Option<PackageSpecifier> m_default;
    };
}

#endif // ZURI_PACKAGER_PACKAGING_CONVERSIONS_H
