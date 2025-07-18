#ifndef ZURI_PACKAGER_PACKAGE_DEPENDENCY_H
#define ZURI_PACKAGER_PACKAGE_DEPENDENCY_H

#include <string>
#include <vector>
#include <tempo_config/config_builder.h>

#include "package_requirement.h"

namespace zuri_packager {

    class PackageDependency {
    public:
        PackageDependency();
        PackageDependency(
            const PackageId &packageId,
            const RequirementsList &requirements);
        PackageDependency(
            const std::string &packageName,
            const std::string &packageDomain,
            const std::vector<std::shared_ptr<AbstractPackageRequirement>> &requirements);
        PackageDependency(const PackageDependency &other);

        bool isValid() const;

        PackageId getPackageId() const;
        RequirementsList getRequirements() const;

        std::string getName() const;
        std::string getDomain() const;

        std::vector<std::shared_ptr<AbstractPackageRequirement>>::const_iterator requirementsBegin() const;
        std::vector<std::shared_ptr<AbstractPackageRequirement>>::const_iterator requirementsEnd() const;
        int numRequirements() const;

        bool satisfiedBy(const PackageSpecifier &specifier) const;

        tempo_config::ConfigNode toNode() const;

    private:
        struct Priv {
            PackageId id;
            RequirementsList requirements;
        };
        std::shared_ptr<Priv> m_priv;
    };
}

#endif // ZURI_PACKAGER_PACKAGE_DEPENDENCY_H
