#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <tempo_config/config_serde.h>
#include <tempo_test/result_matchers.h>
#include <tempo_test/status_matchers.h>

#include <zuri_build/build_graph.h>

TEST(BuildGraph, ConfigureSingleTarget)
{
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

    auto createBuildGraph = BuildGraph::create(targetStore, importStore);
    ASSERT_THAT (createBuildGraph, tempo_test::IsResult());
}

TEST(BuildGraph, ConfigureMultipleTargetsNoDependencies)
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

    auto createBuildGraph = BuildGraph::create(targetStore, importStore);
    ASSERT_THAT (createBuildGraph, tempo_test::IsResult());
}

TEST(BuildGraph, ConfigureMultipleTargetsADependsBDependsC)
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

    auto createBuildGraph = BuildGraph::create(targetStore, importStore);
    ASSERT_THAT (createBuildGraph, tempo_test::IsResult());
}

TEST(BuildGraph, ConfigureMultipleTargetsADependsBAndC)
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

    auto createBuildGraph = BuildGraph::create(targetStore, importStore);
    ASSERT_THAT (createBuildGraph, tempo_test::IsResult());
}


TEST(BuildGraph, DetermineTargetBuildOrder)
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

    auto calculateC = buildGraph->calculateBuildOrder("C");
    ASSERT_THAT (calculateC, tempo_test::IsResult());
    auto orderC = calculateC.getResult();
    ASSERT_THAT  (orderC, testing::ElementsAre("C"));

    auto calculateB = buildGraph->calculateBuildOrder("B");
    ASSERT_THAT (calculateB, tempo_test::IsResult());
    auto orderB = calculateB.getResult();
    ASSERT_THAT  (orderB, testing::ElementsAre("C", "B"));

    auto calculateA = buildGraph->calculateBuildOrder("A");
    ASSERT_THAT (calculateA, tempo_test::IsResult());
    auto orderA = calculateA.getResult();
    ASSERT_THAT  (orderA, testing::ElementsAre("C", "B", "A"));
}

