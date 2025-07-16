
#include <zuri_build/import_resolver.h>
#include <zuri_build/program_status.h>
#include <zuri_distributor/static_package_resolver.h>

ImportResolver::ImportResolver(
    const tempo_config::ConfigMap &resolverConfig,
    std::shared_ptr<zuri_distributor::PackageCache> importPackageCache)
    : m_resolverConfig(resolverConfig),
      m_importPackageCache(std::move(importPackageCache))
{
    TU_ASSERT (m_importPackageCache != nullptr);
}

tempo_utils::Status
ImportResolver::configure()
{
    auto httpResolver = std::make_shared<zuri_distributor::HttpPackageResolver>();
    m_resolvers.push_back(std::move(httpResolver));

    return {};
}

tempo_utils::Status
ImportResolver::addDependency(const zuri_packager::PackageDependency &dependency)
{
    auto packageId = dependency.getPackageId();
    if (m_dependencies.contains(packageId))
        return ProgramStatus::forCondition(ProgramCondition::ProgramError,
            "dependency {} already exists", packageId.toString());
    m_dependencies[packageId] = dependency.getRequirements();
    return {};
}

struct PendingRequirement {
    zuri_packager::PackageId targetId;
};

tempo_utils::Status
ImportResolver::resolve()
{
    zuri_distributor::DependencySet dependencyGraph;
    std::queue<PendingRequirement> pending;

    for (const auto &entry : m_dependencies) {
        zuri_packager::PackageDependency dependency(entry.first, entry.second);
        TU_RETURN_IF_NOT_OK (dependencyGraph.addDependency(dependency));
        
    }
}