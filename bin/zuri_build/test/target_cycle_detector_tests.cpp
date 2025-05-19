#include <absl/strings/str_join.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <tempo_config/config_serde.h>
#include <tempo_test/result_matchers.h>
#include <tempo_test/status_matchers.h>

#include <zuri_build/build_graph.h>

#include "zuri_build/target_cycle_detector.h"

TEST(TargetCycleDetector, SingleTargetNoCycles) {
    tempo_config::ConfigNode targetsConfig;
    TU_ASSIGN_OR_RAISE (targetsConfig, tempo_config::read_config_string(R"(
    {
        "tgt1": {
            "type": "Program",
            "programMain": "/prog"
        }
    }
    )"));
    auto targetStore = std::make_shared<TargetStore>(targetsConfig.toMap());
    TU_RAISE_IF_NOT_OK (targetStore->configure());
    auto importStore = std::make_shared<ImportStore>(tempo_config::ConfigMap{});
    TU_RAISE_IF_NOT_OK (importStore->configure());
    std::shared_ptr<BuildGraph> buildGraph;
    TU_ASSIGN_OR_RAISE (buildGraph, BuildGraph::create(targetStore, importStore));

    auto createTargetCycleDetector = TargetCycleDetector::create(buildGraph);
    ASSERT_THAT (createTargetCycleDetector, tempo_test::IsResult());

    auto targetCycleDetector = createTargetCycleDetector.getResult();
    ASSERT_FALSE (targetCycleDetector->hasCycles());
    ASSERT_EQ (0, targetCycleDetector->numCycles());
}

TEST(TargetCycleDetector, MultipleIndependentTargetsNoCycles)
{
    tempo_config::ConfigNode targetsConfig;
    TU_ASSIGN_OR_RAISE (targetsConfig, tempo_config::read_config_string(R"(
    {
        "A": {
            "type": "Program",
            "programMain": "/prog1"
        },
        "B": {
            "type": "Program",
            "programMain": "/prog2"
        },
        "C": {
            "type": "Program",
            "programMain": "/prog3"
        }
    }
    )"));
    auto targetStore = std::make_shared<TargetStore>(targetsConfig.toMap());
    TU_RAISE_IF_NOT_OK (targetStore->configure());
    auto importStore = std::make_shared<ImportStore>(tempo_config::ConfigMap{});
    TU_RAISE_IF_NOT_OK (importStore->configure());
    std::shared_ptr<BuildGraph> buildGraph;
    TU_ASSIGN_OR_RAISE (buildGraph, BuildGraph::create(targetStore, importStore));

    auto createTargetCycleDetector = TargetCycleDetector::create(buildGraph);
    ASSERT_THAT (createTargetCycleDetector, tempo_test::IsResult());

    auto targetCycleDetector = createTargetCycleDetector.getResult();
    ASSERT_FALSE (targetCycleDetector->hasCycles());
    ASSERT_EQ (0, targetCycleDetector->numCycles());
}

TEST(TargetCycleDetector, MultipleTargetsADependsBDependsCNoCycles)
{
    tempo_config::ConfigNode targetsConfig;
    TU_ASSIGN_OR_RAISE (targetsConfig, tempo_config::read_config_string(R"(
    {
        "A": {
            "type": "Program",
            "programMain": "/prog1",
            "depends": [ "B" ]
        },
        "B": {
            "type": "Library",
            "libraryModules": ["/prog2"],
            "depends": [ "C" ]
        },
        "C": {
            "type": "Library",
            "libraryModules": ["/prog3"]
        }
    }
    )"));
    auto targetStore = std::make_shared<TargetStore>(targetsConfig.toMap());
    TU_RAISE_IF_NOT_OK (targetStore->configure());
    auto importStore = std::make_shared<ImportStore>(tempo_config::ConfigMap{});
    TU_RAISE_IF_NOT_OK (importStore->configure());
    std::shared_ptr<BuildGraph> buildGraph;
    TU_ASSIGN_OR_RAISE (buildGraph, BuildGraph::create(targetStore, importStore));

    auto createTargetCycleDetector = TargetCycleDetector::create(buildGraph);
    ASSERT_THAT (createTargetCycleDetector, tempo_test::IsResult());

    auto targetCycleDetector = createTargetCycleDetector.getResult();
    ASSERT_FALSE (targetCycleDetector->hasCycles());
    ASSERT_EQ (0, targetCycleDetector->numCycles());
}

TEST(TargetCycleDetector, MultipleTargetsADependsBAndCNoCycles)
{
    tempo_config::ConfigNode targetsConfig;
    TU_ASSIGN_OR_RAISE (targetsConfig, tempo_config::read_config_string(R"(
    {
        "A": {
            "type": "Program",
            "programMain": "/prog1",
            "depends": [ "B", "C" ]
        },
        "B": {
            "type": "Library",
            "libraryModules": ["/prog2"],
            "depends": [ "C" ]
        },
        "C": {
            "type": "Library",
            "libraryModules": ["/prog3"]
        }
    }
    )"));
    auto targetStore = std::make_shared<TargetStore>(targetsConfig.toMap());
    TU_RAISE_IF_NOT_OK (targetStore->configure());
    auto importStore = std::make_shared<ImportStore>(tempo_config::ConfigMap{});
    TU_RAISE_IF_NOT_OK (importStore->configure());
    std::shared_ptr<BuildGraph> buildGraph;
    TU_ASSIGN_OR_RAISE (buildGraph, BuildGraph::create(targetStore, importStore));

    auto createTargetCycleDetector = TargetCycleDetector::create(buildGraph);
    ASSERT_THAT (createTargetCycleDetector, tempo_test::IsResult());

    auto targetCycleDetector = createTargetCycleDetector.getResult();
    ASSERT_FALSE (targetCycleDetector->hasCycles());
    ASSERT_EQ (0, targetCycleDetector->numCycles());
}

TEST(TargetCycleDetector, CircularDependency)
{
    tempo_config::ConfigNode targetsConfig;
    TU_ASSIGN_OR_RAISE (targetsConfig, tempo_config::read_config_string(R"(
    {
        "A": {
            "type": "Program",
            "programMain": "/prog1",
            "depends": [ "B" ]
        },
        "B": {
            "type": "Library",
            "libraryModules": ["/prog2"],
            "depends": [ "C" ]
        },
        "C": {
            "type": "Library",
            "libraryModules": ["/prog3"],
            "depends": [ "A" ]
        }
    }
    )"));
    auto targetStore = std::make_shared<TargetStore>(targetsConfig.toMap());
    TU_RAISE_IF_NOT_OK (targetStore->configure());
    auto importStore = std::make_shared<ImportStore>(tempo_config::ConfigMap{});
    TU_RAISE_IF_NOT_OK (importStore->configure());
    std::shared_ptr<BuildGraph> buildGraph;
    TU_ASSIGN_OR_RAISE (buildGraph, BuildGraph::create(targetStore, importStore));

    auto createTargetCycleDetector = TargetCycleDetector::create(buildGraph);
    ASSERT_THAT (createTargetCycleDetector, tempo_test::IsResult());

    auto targetCycleDetector = createTargetCycleDetector.getResult();
    ASSERT_TRUE (targetCycleDetector->hasCycles());
    ASSERT_EQ (1, targetCycleDetector->numCycles());

    auto cycle = *targetCycleDetector->cyclesBegin();
    TU_LOG_ERROR << "cycle: " << absl::StrJoin(cycle, "->");
}
