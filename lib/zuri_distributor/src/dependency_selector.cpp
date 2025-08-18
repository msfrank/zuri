
#include <zuri_distributor/dependency_selector.h>
#include <zuri_distributor/distributor_result.h>
#include <zuri_packager/package_reader.h>

zuri_distributor::DependencySelector::DependencySelector(std::shared_ptr<AbstractPackageResolver> resolver)
    : m_resolver(std::move(resolver))
{
    TU_ASSERT (m_resolver != nullptr);
}

tempo_utils::Status
zuri_distributor::DependencySelector::addDirectDependency(
    const zuri_packager::PackageId &packageId,
    std::string_view shortcut)
{
    PendingSelection pendingSelection;
    pendingSelection.type = PendingSelection::Type::Id;
    pendingSelection.requestedId = packageId;
    pendingSelection.shortcut = shortcut;
    m_pending.push(std::move(pendingSelection));
    return {};
}

tempo_utils::Status
zuri_distributor::DependencySelector::addDirectDependency(
    const zuri_packager::PackageSpecifier &packageSpecifier,
    std::string_view shortcut)
{
    PendingSelection pendingSelection;
    pendingSelection.type = PendingSelection::Type::Specifier;
    pendingSelection.requestedSpecifier = packageSpecifier;
    pendingSelection.shortcut = shortcut;
    m_pending.push(std::move(pendingSelection));
    return {};
}

tempo_utils::Status
zuri_distributor::DependencySelector::addDirectDependency(
    const std::filesystem::path &packagePath,
    std::string_view shortcut)
{
    PendingSelection pendingSelection;
    pendingSelection.type = PendingSelection::Type::Path;
    pendingSelection.requestedPath = packagePath;
    pendingSelection.shortcut = shortcut;
    m_pending.push(std::move(pendingSelection));
    return {};
}

tempo_utils::Status
zuri_distributor::DependencySelector::dependOnLatestVersion(
    const zuri_packager::PackageId &id,
    const std::string &shortcut)
{
    CollectionDescriptor collectionDescriptor;
    TU_ASSIGN_OR_RETURN (collectionDescriptor, m_resolver->getCollection(id));

    zuri_packager::PackageVersion latestVersion;
    for (const auto &entry : collectionDescriptor.versions) {
        if (!latestVersion.isValid() || entry.first > latestVersion) {
            if (!entry.second.pruned) {
                latestVersion = entry.first;
            }
        }
    }

    if (!latestVersion.isValid())
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "no usable version found for '{}'", id.toString());
    zuri_packager::PackageSpecifier specifier(id, latestVersion);

    return dependOnSpecifiedVersion(specifier, shortcut);
}

tempo_utils::Status
zuri_distributor::DependencySelector::dependOnSpecifiedVersion(
    const zuri_packager::PackageSpecifier &specifier,
    const std::string &shortcut)
{
    PackageDescriptor packageDescriptor;
    TU_ASSIGN_OR_RETURN (packageDescriptor, m_resolver->getPackage(
        specifier.getPackageId(), specifier.getPackageVersion()));

    TU_RETURN_IF_NOT_OK (m_dependencies.addDirectDependency(specifier, shortcut));

    for (const auto &requested : packageDescriptor.dependencies) {
        PendingSelection pendingSelection;
        pendingSelection.type = PendingSelection::Type::Transitive;
        pendingSelection.requestedSpecifier = requested;
        pendingSelection.target = specifier;
        m_pending.push(std::move(pendingSelection));
    }
    m_packageUrls[specifier] = packageDescriptor.url;

    return {};
}

tempo_utils::Status
zuri_distributor::DependencySelector::dependOnSpecifiedPath(
    const std::filesystem::path &path,
    const std::string &shortcut)
{
    std::shared_ptr<zuri_packager::PackageReader> reader;
    TU_ASSIGN_OR_RETURN (reader, zuri_packager::PackageReader::open(path));

    zuri_packager::PackageSpecifier specifier;
    TU_ASSIGN_OR_RETURN (specifier, reader->readPackageSpecifier());

    zuri_packager::RequirementsMap requirements;
    TU_ASSIGN_OR_RETURN (requirements, reader->readRequirementsMap());

    TU_RETURN_IF_NOT_OK (m_dependencies.addDirectDependency(specifier, shortcut));

    for (auto it = requirements.requirementsBegin(); it != requirements.requirementsEnd(); it++) {
        zuri_packager::PackageSpecifier requested(it->first, it->second);
        PendingSelection pendingSelection;
        pendingSelection.type = PendingSelection::Type::Transitive;
        pendingSelection.requestedSpecifier = requested;
        pendingSelection.target = specifier;
        m_pending.push(std::move(pendingSelection));
    }
    m_packageUrls[specifier] = tempo_utils::Url::fromFilesystemPath(path);

    return {};
}
tempo_utils::Status
zuri_distributor::DependencySelector::dependTransitively(
    const zuri_packager::PackageSpecifier &target,
    const zuri_packager::PackageSpecifier &dependency)
{
    bool newSelection;
    TU_ASSIGN_OR_RETURN (newSelection, m_dependencies.addTransitiveDependency(target, dependency));
    if (!newSelection)
        return {};

    PackageDescriptor packageDescriptor;
    TU_ASSIGN_OR_RETURN (packageDescriptor, m_resolver->getPackage(
        dependency.getPackageId(), dependency.getPackageVersion()));

    for (const auto &requested : packageDescriptor.dependencies) {
        PendingSelection pendingSelection;
        pendingSelection.type = PendingSelection::Type::Transitive;
        pendingSelection.requestedSpecifier = requested;
        pendingSelection.target = dependency;
        m_pending.push(std::move(pendingSelection));
    }

    m_packageUrls[dependency] = packageDescriptor.url;

    return {};
}

tempo_utils::Status
zuri_distributor::DependencySelector::selectDependencies()
{
    while (!m_pending.empty()) {
        PendingSelection curr = m_pending.front();
        m_pending.pop();

        switch (curr.type) {
            case PendingSelection::Type::Id:
                TU_RETURN_IF_NOT_OK (dependOnLatestVersion(curr.requestedId, curr.shortcut));
                break;
            case PendingSelection::Type::Specifier:
                TU_RETURN_IF_NOT_OK (dependOnSpecifiedVersion(curr.requestedSpecifier, curr.shortcut));
                break;
            case PendingSelection::Type::Path:
                TU_RETURN_IF_NOT_OK (dependOnSpecifiedPath(curr.requestedPath, curr.shortcut));
                break;
            case PendingSelection::Type::Transitive:
                TU_RETURN_IF_NOT_OK (dependTransitively(curr.target, curr.requestedSpecifier));
                break;
            default:
                return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
                    "invalid pending selection");
        }
    }
    return {};
}

tempo_utils::Result<std::vector<zuri_distributor::Selection>>
zuri_distributor::DependencySelector::calculateDependencyOrder()
{
    TU_RETURN_IF_NOT_OK (selectDependencies());

    std::vector<Dependency> resolutionOrder;
    TU_ASSIGN_OR_RETURN (resolutionOrder, m_dependencies.calculateResolutionOrder());

    std::vector<Selection> dependencyOrder;
    for (const auto &dependency : resolutionOrder) {
        auto entry = m_packageUrls.find(dependency.specifier);
        if (entry == m_packageUrls.cend())
            return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
                "missing package url for {}", dependency.specifier.toString());
        const auto &packageUrl = entry->second;

        Selection selection;
        selection.specifier = dependency.specifier;
        selection.url = packageUrl;
        selection.shortcut = dependency.shortcut;
        dependencyOrder.push_back(std::move(selection));
    }

    return dependencyOrder;
}

