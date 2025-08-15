#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_test/lyric_tester.h>
#include <tempo_config/config_utils.h>
#include <tempo_test/result_matchers.h>
#include <tempo_test/status_matchers.h>
#include <tempo_utils/tempdir_maker.h>

#include <zuri_build/build_graph.h>
#include <zuri_build/target_builder.h>

#include "zuri_build/collect_modules_task.h"

class TargetBuilderTests : public ::testing::Test {
protected:
    std::unique_ptr<tempo_utils::TempdirMaker> installRoot;

    void SetUp() override {
        installRoot = std::make_unique<tempo_utils::TempdirMaker>("install.XXXXXXXX");
        TU_RAISE_IF_NOT_OK (installRoot->getStatus());
    }
    void TearDown() override {
        if (installRoot) {
            std::filesystem::remove_all(installRoot->getTempdir());
            installRoot.reset();
        }
    }
};

TEST_F(TargetBuilderTests, BuildLibrary)
{
    lyric_test::TesterOptions testerOptions;
    testerOptions.taskRegistry = std::make_shared<lyric_build::TaskRegistry>();
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
    auto targetStore = std::make_shared<zuri_tooling::TargetStore>(targetsConfig.toMap());
    TU_RAISE_IF_NOT_OK (targetStore->configure());
    auto importStore = std::make_shared<zuri_tooling::ImportStore>(tempo_config::ConfigMap{});
    TU_RAISE_IF_NOT_OK (importStore->configure());
    std::shared_ptr<BuildGraph> buildGraph;
    TU_ASSIGN_OR_RAISE (buildGraph, BuildGraph::create(targetStore, importStore));

    auto tempdir = installRoot->getTempdir();

    auto *testRunner = tester.getRunner();
    auto *builder = testRunner->getBuilder();
    auto shortcutResolver = std::make_shared<lyric_importer::ShortcutResolver>();
    std::shared_ptr<zuri_distributor::PackageCache> packageCache;
    TU_ASSIGN_OR_RAISE (packageCache, zuri_distributor::PackageCache::openOrCreate(tempdir, "pkgcache"));

    TargetBuilder targetBuilder(buildGraph, builder, shortcutResolver, packageCache, installRoot->getTempdir());

    auto buildTargetResult = targetBuilder.buildTarget("lib1", {});
    ASSERT_THAT (buildTargetResult, tempo_test::IsResult());
}
