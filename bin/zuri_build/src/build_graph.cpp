
#include <absl/strings/str_join.h>

#include <zuri_build/build_graph.h>

BuildGraph::BuildGraph(
    std::shared_ptr<TargetStore> targetStore,
    std::shared_ptr<ImportStore> importStore,
    std::unique_ptr<BuildGraphImpl> buildGraph,
    absl::flat_hash_map<std::string,BuildGraphVertex> targetsMap,
    absl::flat_hash_map<std::string,BuildGraphVertex> importsMap,
    absl::flat_hash_set<tempo_utils::Url> requestedPackages,
    absl::flat_hash_set<std::string> requestedRequirements)
    : m_targetStore(std::move(targetStore)),
      m_importStore(std::move(importStore)),
      m_buildGraph(std::move(buildGraph)),
      m_targetsMap(std::move(targetsMap)),
      m_importsMap(std::move(importsMap)),
      m_requestedPackages(std::move(requestedPackages)),
      m_requestedRequirements(std::move(requestedRequirements))
{
    TU_ASSERT (m_targetStore != nullptr);
    TU_ASSERT (m_importStore != nullptr);
    TU_ASSERT (m_buildGraph != nullptr);
}

tempo_utils::Result<std::shared_ptr<BuildGraph>>
BuildGraph::create(
    std::shared_ptr<TargetStore> targetStore,
    std::shared_ptr<ImportStore> importStore)
{
    auto buildGraph = std::make_unique<BuildGraphImpl>();

    auto target_name = get(vertex_target_name_t(), *buildGraph);

    absl::flat_hash_map<std::string,BuildGraphVertex> targetsMap;
    absl::flat_hash_map<std::string,BuildGraphVertex> importsMap;
    absl::flat_hash_set<tempo_utils::Url> requestedPackages;
    absl::flat_hash_set<std::string> requestedRequirements;

    // construct map of target name to dependency index
    for (auto it = targetStore->targetsBegin(); it != targetStore->targetsEnd(); it++) {
        auto v = boost::add_vertex(*buildGraph);
        boost::put(target_name, v, it->first);
        targetsMap[it->first] = v;
    }

    // construct map of import name to dependency index (if import refers to a target) and additionally
    // build the sets of requested packages and package requirements
    for (auto it = importStore->importsBegin(); it != importStore->importsEnd(); it++) {
        const auto &importName = it->first;
        const auto &importEntry = it->second;
        switch (importEntry.type) {
            case ImportEntryType::Target: {
                auto entry = targetsMap.find(importEntry.targetName);
                if (entry == targetsMap.cend())
                    return tempo_config::ConfigStatus::forCondition(
                        tempo_config::ConfigCondition::kConfigInvariant,
                        "import '{}' refers to nonexistent target '{}'", importName, importEntry.targetName);
                importsMap[importName] = entry->second;
                break;
            }
            case ImportEntryType::Requirement: {
                requestedRequirements.insert(importEntry.requirementSpec);
                break;
            }
            case ImportEntryType::Package: {
                requestedPackages.insert(importEntry.packageUrl);
                break;
            }
            default:
                return tempo_config::ConfigStatus::forCondition(
                    tempo_config::ConfigCondition::kConfigInvariant,
                    "invalid type for import '{}'", importName);
        }
    }

    // build list of dependency edges
    for (auto it = targetStore->targetsBegin(); it != targetStore->targetsEnd(); it++) {
        const auto &targetName = it->first;
        const auto &targetEntry = it->second;
        auto targetVertex = targetsMap.at(it->first);

        for (const auto &targetDep : targetEntry.depends) {
            auto entry = targetsMap.find(targetDep);
            if (entry == targetsMap.cend())
                return tempo_config::ConfigStatus::forCondition(
                    tempo_config::ConfigCondition::kConfigInvariant,
                    "target '{}' refers to nonexistent target dependency '{}'", targetName, targetDep);
            boost::add_edge(entry->second, targetVertex, *buildGraph);
        }

        for (const auto &targetImport : targetEntry.imports) {
            if (!importStore->hasImport(targetImport))
                return tempo_config::ConfigStatus::forCondition(
                    tempo_config::ConfigCondition::kConfigInvariant,
                    "target '{}' refers to nonexistent import '{}'", targetName, targetImport);
            const auto &importEntry = importStore->getImport(targetImport);
            switch (importEntry.type) {
                case ImportEntryType::Target: {
                    boost::add_edge(importsMap.at(targetImport), targetVertex, *buildGraph);
                    break;
                }
                default:
                    break;
            }
        }
    }

    return std::shared_ptr<BuildGraph>(new BuildGraph(
        std::move(targetStore),
        std::move(importStore),
        std::move(buildGraph),
        std::move(targetsMap),
        std::move(importsMap),
        std::move(requestedPackages),
        std::move(requestedRequirements)));
}