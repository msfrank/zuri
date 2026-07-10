#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_test/lyric_tester.h>
#include <tempo_config/config_utils.h>
#include <tempo_test/result_matchers.h>
#include <tempo_utils/tempdir_maker.h>
#include <zuri_build/target_builder.h>
#include <zuri_tooling/build_graph.h>

#include "base_build_fixture.h"

class TargetBuilderTests : public BaseBuildFixture {};

TEST_F(TargetBuilderTests, BuildLibrary)
{
    lyric_test::TesterOptions testerOptions;
    testerOptions.taskRegistry = taskRegistry;

    lyric_test::LyricTester tester(testerOptions);
    TU_RAISE_IF_NOT_OK (tester.configure());

    lyric_common::ModuleLocation mod1;
    TU_ASSIGN_OR_RAISE (mod1, tester.writeModule("def FortyTwo(): I64 { 42 }", "mod1"));

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
    auto targetStore = std::make_shared<zuri_tooling::TargetStore>(targetsConfig.toMap());
    TU_RAISE_IF_NOT_OK (targetStore->configure());
    auto importStore = std::make_shared<zuri_tooling::ImportStore>(tempo_config::ConfigMap{});
    TU_RAISE_IF_NOT_OK (importStore->configure());
    std::shared_ptr<zuri_tooling::BuildGraph> buildGraph;
    TU_ASSIGN_OR_RAISE (buildGraph, zuri_tooling::BuildGraph::create(targetStore, importStore));

    auto tempdir = installRoot->getTempdir();
    auto *testRunner = tester.getRunner();
    auto *builder = testRunner->getBuilder();

    zuri_build::TargetBuilder targetBuilder(runtime, buildGraph, builder, {}, tempdir);

    auto buildTargetResult = targetBuilder.buildTarget("lib1");
    ASSERT_THAT (buildTargetResult, tempo_test::IsResult());
}
