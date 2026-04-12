#ifndef ZURI_BUILD_BASE_BUILD_FIXTURE_H
#define ZURI_BUILD_BASE_BUILD_FIXTURE_H

#include <gtest/gtest.h>

#include <lyric_build/task_registry.h>
#include <tempo_utils/tempdir_maker.h>
#include <zuri_distributor/runtime.h>

class BaseBuildFixture : public ::testing::Test {
protected:
    std::unique_ptr<tempo_utils::TempdirMaker> installRoot;
    std::shared_ptr<zuri_distributor::Runtime> runtime;
    std::shared_ptr<lyric_build::TaskRegistry> taskRegistry;

    void SetUp() override;
    void TearDown() override;
};

#endif // ZURI_BUILD_BASE_BUILD_FIXTURE_H
