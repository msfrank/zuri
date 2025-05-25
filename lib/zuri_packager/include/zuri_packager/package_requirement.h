#ifndef ZURI_PACKAGER_PACKAGE_REQUIREMENT_H
#define ZURI_PACKAGER_PACKAGE_REQUIREMENT_H

#include <tempo_config/config_builder.h>

#include "package_specifier.h"

namespace zuri_packager {

    class AbstractPackageRequirement {
    public:
        virtual ~AbstractPackageRequirement() = default;

        virtual bool satisfiedBy(const PackageSpecifier &specifier) const = 0;

        virtual tempo_config::ConfigNode toNode() const = 0;
    };

    enum class VersionComparison {
        Invalid,
        Equal,
        NotEqual,
        GreaterThan,
        GreaterOrEqual,
        LesserThan,
        LesserOrEqual,
    };

    class VersionRequirement : public AbstractPackageRequirement {
    public:
        static std::shared_ptr<AbstractPackageRequirement> create(
            const PackageSpecifier &specifier,
            VersionComparison comparison);

        bool satisfiedBy(const PackageSpecifier &specifier) const override;

        tempo_config::ConfigNode toNode() const override;

    private:
        PackageSpecifier m_specifier;
        VersionComparison m_comparison;

        VersionRequirement(const PackageSpecifier &specifier, VersionComparison comparison);
    };

}

#endif // ZURI_PACKAGER_PACKAGE_REQUIREMENT_H
