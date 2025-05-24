#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_test/lyric_tester.h>
#include <tempo_config/config_serde.h>
#include <tempo_test/result_matchers.h>
#include <tempo_test/status_matchers.h>

#include <zuri_build/build_graph.h>
#include <zuri_build/target_builder.h>

#include "zuri_build/collect_modules_task.h"

TEST(TargetBuilder, BuildLibrary) {
    lyric_test::TesterOptions testerOptions;
    testerOptions.taskRegistry = std::make_shared<lyric_build::TaskRegistry>();
    testerOptions.taskSettings = lyric_build::TaskSettings(tempo_config::ConfigMap{
        {"global", tempo_config::ConfigMap{
                {"sourceBasePath", tempo_config::ConfigValue("src")},
        }}
    });
    TU_RAISE_IF_NOT_OK (testerOptions.taskRegistry->registerTaskDomain("collect_modules", new_collect_modules_task));
    lyric_test::LyricTester tester(testerOptions);
    TU_RAISE_IF_NOT_OK (tester.configure());

    lyric_common::ModuleLocation mod1;
    TU_ASSIGN_OR_RAISE (mod1, tester.writeModule("def FortyTwo(): Int { 42 }", "mod1"));

    tempo_config::ConfigNode targetsConfig;
    TU_ASSIGN_OR_RAISE (targetsConfig, tempo_config::read_config_string(R"(
    {
        "lib1": {
            "type": "Library",
            "specifier": "lib1-0.0.1@foo.corp",
            "libraryModules": ["/mod1"]
        }
    }
    )"));
    auto targetStore = std::make_shared<TargetStore>(targetsConfig.toMap());
    TU_RAISE_IF_NOT_OK (targetStore->configure());
    auto importStore = std::make_shared<ImportStore>(tempo_config::ConfigMap{});
    TU_RAISE_IF_NOT_OK (importStore->configure());
    std::shared_ptr<BuildGraph> buildGraph;
    TU_ASSIGN_OR_RAISE (buildGraph, BuildGraph::create(targetStore, importStore));

    auto *testRunner = tester.getRunner();
    auto *builder = testRunner->getBuilder();
    TargetBuilder targetBuilder(buildGraph, builder, std::filesystem::current_path());

    auto buildTargetResult = targetBuilder.buildTarget("lib1");
    ASSERT_THAT (buildTargetResult, tempo_test::IsResult());
}
