
#include <zuri_build/collect_modules_task.h>
#include <zuri_build/provide_object_task.h>
#include <zuri_build/provide_plugin_task.h>

#include "base_build_fixture.h"

void
BaseBuildFixture::SetUp()
{
    installRoot = std::make_unique<tempo_utils::TempdirMaker>(
        std::filesystem::current_path(), "install.XXXXXXXX");
    TU_RAISE_IF_NOT_OK (installRoot->getStatus());

    auto runtimeDirectory = installRoot->getTempdir() / "runtime";
    std::filesystem::create_directory(runtimeDirectory);
    TU_ASSIGN_OR_RAISE (runtime, zuri_distributor::Runtime::openOrCreate(runtimeDirectory));

    taskRegistry = std::make_shared<lyric_build::TaskRegistry>();
    TU_RAISE_IF_NOT_OK (taskRegistry->registerTaskDomain(
        "collect_modules", zuri_build::new_collect_modules_task));
    TU_RAISE_IF_NOT_OK (taskRegistry->registerTaskDomain(
        "provide_object", zuri_build::new_provide_object_task));
    TU_RAISE_IF_NOT_OK (taskRegistry->registerTaskDomain(
        "provide_plugin", zuri_build::new_provide_plugin_task));
}

void
BaseBuildFixture::TearDown()
{
    if (installRoot) {
        std::filesystem::remove_all(installRoot->getTempdir());
        installRoot.reset();
    }
}