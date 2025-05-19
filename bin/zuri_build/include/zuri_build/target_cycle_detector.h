#ifndef ZURI_BUILD_TARGET_CYCLE_DETECTOR_H
#define ZURI_BUILD_TARGET_CYCLE_DETECTOR_H

#include "build_graph.h"

class TargetCycleDetector {
public:
    static tempo_utils::Result<std::shared_ptr<TargetCycleDetector>> create(std::shared_ptr<BuildGraph> buildGraph);

    bool hasCycles() const;
    absl::flat_hash_set<std::vector<std::string>>::const_iterator cyclesBegin() const;
    absl::flat_hash_set<std::vector<std::string>>::const_iterator cyclesEnd() const;
    int numCycles() const;

private:
    std::shared_ptr<BuildGraph> m_buildGraph;
    absl::flat_hash_set<std::vector<std::string>> m_targetCycles;

    TargetCycleDetector(
        std::shared_ptr<BuildGraph> buildGraph,
        absl::flat_hash_set<std::vector<std::string>> targetCycles);
};

#endif //ZURI_BUILD_TARGET_CYCLE_DETECTOR_H
