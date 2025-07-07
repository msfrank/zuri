#ifndef ZURI_PACKAGER_PACKAGE_REQUIREMENT_H
#define ZURI_PACKAGER_PACKAGE_REQUIREMENT_H

#include <tempo_config/config_builder.h>

#include "package_specifier.h"
#include "package_types.h"

namespace zuri_packager {

    enum class RequirementType {
        Invalid,
        ExactVersion,
        VersionRange,
    };

    struct VersionInterval {
        PackageVersion closedLowerBound;
        PackageVersion openUpperBound;
    };

    class AbstractPackageRequirement {
    public:
        virtual ~AbstractPackageRequirement() = default;

        virtual RequirementType getType() const = 0;

        virtual bool satisfiedBy(const PackageVersion &version) const = 0;

        virtual VersionInterval getInterval() const = 0;

        virtual tempo_config::ConfigNode toNode() const = 0;
    };

    class ExactVersionRequirement : public AbstractPackageRequirement {
    public:
        static std::shared_ptr<AbstractPackageRequirement> create(const PackageVersion &packageVersion);

        RequirementType getType() const override;
        VersionInterval getInterval() const override;
        bool satisfiedBy(const PackageVersion &version) const override;

        tempo_config::ConfigNode toNode() const override;

    private:
        PackageVersion m_version;

        explicit ExactVersionRequirement(const PackageVersion &version);
    };

    class VersionRangeRequirement : public AbstractPackageRequirement {
    public:
        static std::shared_ptr<AbstractPackageRequirement> create(
            const PackageVersion &lowerBound,
            const PackageVersion &upperBound);

        RequirementType getType() const override;
        VersionInterval getInterval() const override;
        bool satisfiedBy(const PackageVersion &version) const override;

        tempo_config::ConfigNode toNode() const override;

    private:
        PackageVersion m_lowerBound;
        PackageVersion m_upperBound;

        VersionRangeRequirement(const PackageVersion &lower, const PackageVersion &upper);
    };

    class RequirementsList {
    public:
        RequirementsList();
        explicit RequirementsList(const std::vector<std::shared_ptr<AbstractPackageRequirement>> &requirements);
        RequirementsList(const RequirementsList &other);

        std::vector<std::shared_ptr<AbstractPackageRequirement>>::const_iterator requirementsBegin() const;
        std::vector<std::shared_ptr<AbstractPackageRequirement>>::const_iterator requirementsEnd() const;
        int numRequirements() const;

    private:
        struct Priv {
            std::vector<std::shared_ptr<AbstractPackageRequirement>> requirements;
        };
        std::shared_ptr<Priv> m_priv;
    };
}

#endif // ZURI_PACKAGER_PACKAGE_REQUIREMENT_H
