#ifndef ZURI_PACKAGER_PACKAGE_DEPENDENCY_H
#define ZURI_PACKAGER_PACKAGE_DEPENDENCY_H

#include <string>
#include <vector>

#include "package_requirement.h"

namespace zuri_packager {

    class PackageDependency {
    public:
        PackageDependency();
        PackageDependency(
            const std::string &name,
            const std::string &domain,
            const std::vector<std::shared_ptr<AbstractPackageRequirement>> &requirements);
        PackageDependency(const PackageDependency &other);

        bool isValid() const;

        std::string getName() const;
        std::string getDomain() const;

        std::vector<std::shared_ptr<AbstractPackageRequirement>>::const_iterator requirementsBegin() const;
        std::vector<std::shared_ptr<AbstractPackageRequirement>>::const_iterator requirementsEnd() const;
        int numRequirements() const;

        bool satisfiedBy(const PackageSpecifier &specifier) const;

    private:
        struct Priv {
            std::string name;
            std::string domain;
            std::vector<std::shared_ptr<AbstractPackageRequirement>> requirements;
        };
        std::shared_ptr<Priv> m_priv;
    };
}

#endif // ZURI_PACKAGER_PACKAGE_DEPENDENCY_H
