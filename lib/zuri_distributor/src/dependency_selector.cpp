
#include <zuri_distributor/dependency_selector.h>

zuri_distributor::DependencySelector::DependencySelector(std::shared_ptr<AbstractPackageResolver> resolver)
    : m_resolver(std::move(resolver))
{
    TU_ASSERT (m_resolver != nullptr);
}

tempo_utils::Status
zuri_distributor::DependencySelector::addDirectDependency(const zuri_packager::PackageSpecifier &dependency)
{
    TU_RETURN_IF_NOT_OK (m_dependencies.addDirectDependency(dependency));

    PackageVersionDescriptor packageVersionDescriptor;
    TU_ASSIGN_OR_RETURN (packageVersionDescriptor, m_resolver->describePackageVersion(
        dependency.getPackageId(), dependency.getPackageVersion()));

    for (const auto &requested : packageVersionDescriptor.dependencies) {
        PendingSelection pendingSelection;
        pendingSelection.requested = requested;
        pendingSelection.target = dependency;
        m_pending.push(std::move(pendingSelection));
    }

    return {};
}

tempo_utils::Status
zuri_distributor::DependencySelector::resolveTransitiveDependencies()
{
    while (!m_pending.empty()) {
        PendingSelection curr = m_pending.front();
        m_pending.pop();

        bool newSelection;
        TU_ASSIGN_OR_RETURN (newSelection, m_dependencies.addTransitiveDependency(
            curr.target, curr.requested));
        if (!newSelection)
            continue;

        auto target = curr.requested;

        PackageVersionDescriptor packageVersionDescriptor;
        TU_ASSIGN_OR_RETURN (packageVersionDescriptor, m_resolver->describePackageVersion(
            target.getPackageId(), target.getPackageVersion()));

        for (const auto &requested : packageVersionDescriptor.dependencies) {
            PendingSelection next;
            next.requested = requested;
            next.target = target;
            m_pending.push(std::move(next));
        }
    }
    return {};
}

tempo_utils::Result<std::vector<zuri_packager::PackageSpecifier>>
zuri_distributor::DependencySelector::calculateDependencyOrder()
{
    TU_RETURN_IF_NOT_OK (resolveTransitiveDependencies());
    return m_dependencies.calculateResolutionOrder();
}

