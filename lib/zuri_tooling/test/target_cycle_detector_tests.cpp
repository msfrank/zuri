#include <absl/strings/str_join.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <tempo_config/config_utils.h>
#include <tempo_test/result_matchers.h>
#include <tempo_test/status_matchers.h>
#include <zuri_tooling/build_graph.h>

TEST(TargetCycleDetector, SingleTargetNoCycles) {
    tempo_config::ConfigNode targetsConfig;
    TU_ASSIGN_OR_RAISE (targetsConfig, tempo_config::read_config_string(R"(
    {
        "tgt1": {
            "type": "Program",
            "specifier": "prog-1.0.1@foo.corp",
            "programMain": "/prog"
        }
    }
    )"));
    auto targetStore = std::make_shared<zuri_tooling::TargetStore>(targetsConfig.toMap());
    TU_RAISE_IF_NOT_OK (targetStore->configure());
    auto importStore = std::make_shared<zuri_tooling::ImportStore>(tempo_config::ConfigMap{});
    TU_RAISE_IF_NOT_OK (importStore->configure());
    std::shared_ptr<zuri_tooling::BuildGraph> buildGraph;
    TU_ASSIGN_OR_RAISE (buildGraph, zuri_tooling::BuildGraph::create(targetStore, importStore));

    ASSERT_FALSE (buildGraph->hasCycles());
    ASSERT_EQ (0, buildGraph->numCycles());
}

TEST(TargetCycleDetector, MultipleIndependentTargetsNoCycles)
{
    tempo_config::ConfigNode targetsConfig;
    TU_ASSIGN_OR_RAISE (targetsConfig, tempo_config::read_config_string(R"(
    {
        "A": {
            "type": "Program",
            "specifier": "prog1-1.0.1@foo.corp",
            "programMain": "/prog1"
        },
        "B": {
            "type": "Program",
            "specifier": "prog2-1.0.1@foo.corp",
            "programMain": "/prog2"
        },
        "C": {
            "type": "Program",
            "specifier": "prog3-1.0.1@foo.corp",
            "programMain": "/prog3"
        }
    }
    )"));
    auto targetStore = std::make_shared<zuri_tooling::TargetStore>(targetsConfig.toMap());
    TU_RAISE_IF_NOT_OK (targetStore->configure());
    auto importStore = std::make_shared<zuri_tooling::ImportStore>(tempo_config::ConfigMap{});
    TU_RAISE_IF_NOT_OK (importStore->configure());
    std::shared_ptr<zuri_tooling::BuildGraph> buildGraph;
    TU_ASSIGN_OR_RAISE (buildGraph, zuri_tooling::BuildGraph::create(targetStore, importStore));

    ASSERT_FALSE (buildGraph->hasCycles());
    ASSERT_EQ (0, buildGraph->numCycles());
}

TEST(TargetCycleDetector, MultipleTargetsADependsBDependsCNoCycles)
{
    tempo_config::ConfigNode targetsConfig;
    TU_ASSIGN_OR_RAISE (targetsConfig, tempo_config::read_config_string(R"(
    {
        "A": {
            "type": "Program",
            "specifier": "prog1-1.0.1@foo.corp",
            "programMain": "/prog1",
            "depends": [ "B" ]
        },
        "B": {
            "type": "Library",
            "specifier": "lib1-1.0.1@foo.corp",
            "libraryModules": ["/lib1"],
            "depends": [ "C" ]
        },
        "C": {
            "type": "Library",
            "specifier": "lib2-1.0.1@foo.corp",
            "libraryModules": ["/lib2"]
        }
    }
    )"));
    auto targetStore = std::make_shared<zuri_tooling::TargetStore>(targetsConfig.toMap());
    TU_RAISE_IF_NOT_OK (targetStore->configure());
    auto importStore = std::make_shared<zuri_tooling::ImportStore>(tempo_config::ConfigMap{});
    TU_RAISE_IF_NOT_OK (importStore->configure());
    std::shared_ptr<zuri_tooling::BuildGraph> buildGraph;
    TU_ASSIGN_OR_RAISE (buildGraph, zuri_tooling::BuildGraph::create(targetStore, importStore));

    ASSERT_FALSE (buildGraph->hasCycles());
    ASSERT_EQ (0, buildGraph->numCycles());
}

TEST(TargetCycleDetector, MultipleTargetsADependsBAndCNoCycles)
{
    tempo_config::ConfigNode targetsConfig;
    TU_ASSIGN_OR_RAISE (targetsConfig, tempo_config::read_config_string(R"(
    {
        "A": {
            "type": "Program",
            "specifier": "prog1-1.0.1@foo.corp",
            "programMain": "/prog1",
            "depends": [ "B", "C" ]
        },
        "B": {
            "type": "Library",
            "specifier": "lib1-1.0.1@foo.corp",
            "libraryModules": ["/lib1"],
            "depends": [ "C" ]
        },
        "C": {
            "type": "Library",
            "specifier": "lib2-1.0.1@foo.corp",
            "libraryModules": ["/lib2"]
        }
    }
    )"));
    auto targetStore = std::make_shared<zuri_tooling::TargetStore>(targetsConfig.toMap());
    TU_RAISE_IF_NOT_OK (targetStore->configure());
    auto importStore = std::make_shared<zuri_tooling::ImportStore>(tempo_config::ConfigMap{});
    TU_RAISE_IF_NOT_OK (importStore->configure());
    std::shared_ptr<zuri_tooling::BuildGraph> buildGraph;
    TU_ASSIGN_OR_RAISE (buildGraph, zuri_tooling::BuildGraph::create(targetStore, importStore));

    ASSERT_FALSE (buildGraph->hasCycles());
    ASSERT_EQ (0, buildGraph->numCycles());
}

TEST(TargetCycleDetector, CircularDependency)
{
    tempo_config::ConfigNode targetsConfig;
    TU_ASSIGN_OR_RAISE (targetsConfig, tempo_config::read_config_string(R"(
    {
        "A": {
            "type": "Program",
            "specifier": "prog1-1.0.1@foo.corp",
            "programMain": "/prog1",
            "depends": [ "B" ]
        },
        "B": {
            "type": "Library",
            "specifier": "lib1-1.0.1@foo.corp",
            "libraryModules": ["/lib1"],
            "depends": [ "C" ]
        },
        "C": {
            "type": "Library",
            "specifier": "lib2-1.0.1@foo.corp",
            "libraryModules": ["/lib2"],
            "depends": [ "A" ]
        }
    }
    )"));
    auto targetStore = std::make_shared<zuri_tooling::TargetStore>(targetsConfig.toMap());
    TU_RAISE_IF_NOT_OK (targetStore->configure());
    auto importStore = std::make_shared<zuri_tooling::ImportStore>(tempo_config::ConfigMap{});
    TU_RAISE_IF_NOT_OK (importStore->configure());
    std::shared_ptr<zuri_tooling::BuildGraph> buildGraph;
    TU_ASSIGN_OR_RAISE (buildGraph, zuri_tooling::BuildGraph::create(targetStore, importStore));

    ASSERT_TRUE (buildGraph->hasCycles());
    ASSERT_EQ (1, buildGraph->numCycles());

    auto cycle = *buildGraph->cyclesBegin();
    TU_LOG_ERROR << "cycle: " << absl::StrJoin(cycle, " depends on ");
}

TEST(TargetCycleDetector, MultipleCircularDependencies)
{
    tempo_config::ConfigNode targetsConfig;
    TU_ASSIGN_OR_RAISE (targetsConfig, tempo_config::read_config_string(R"(
    {
        "A": {
            "type": "Program",
            "specifier": "prog1-1.0.1@foo.corp",
            "programMain": "/prog1",
            "depends": [ "B", "C" ]
        },
        "B": {
            "type": "Library",
            "specifier": "lib1-1.0.1@foo.corp",
            "libraryModules": ["/lib1"],
            "depends": [ "C" ]
        },
        "C": {
            "type": "Library",
            "specifier": "lib2-1.0.1@foo.corp",
            "libraryModules": ["/lib2"],
            "depends": [ "A" ]
        }
    }
    )"));
    auto targetStore = std::make_shared<zuri_tooling::TargetStore>(targetsConfig.toMap());
    TU_RAISE_IF_NOT_OK (targetStore->configure());
    auto importStore = std::make_shared<zuri_tooling::ImportStore>(tempo_config::ConfigMap{});
    TU_RAISE_IF_NOT_OK (importStore->configure());
    std::shared_ptr<zuri_tooling::BuildGraph> buildGraph;
    TU_ASSIGN_OR_RAISE (buildGraph, zuri_tooling::BuildGraph::create(targetStore, importStore));

    std::vector targetCycles(buildGraph->cyclesBegin(), buildGraph->cyclesEnd());
    ASSERT_TRUE (buildGraph->hasCycles());
    ASSERT_EQ (2, buildGraph->numCycles());

    auto cycle = *buildGraph->cyclesBegin();
    TU_LOG_ERROR << "cycle: " << absl::StrJoin(cycle, " depends on ");
}
