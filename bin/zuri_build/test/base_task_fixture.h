#ifndef ZURI_BUILD_BASE_TASK_FIXTURE_H
#define ZURI_BUILD_BASE_TASK_FIXTURE_H

#include <gtest/gtest.h>

#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/local_filesystem.h>
#include <lyric_build/task_settings.h>
#include <lyric_build/temp_directory.h>
#include <tempo_tracing/trace_recorder.h>

class BaseTaskFixture : public ::testing::Test {
protected:
    lyric_build::TaskSettings taskSettings;
    lyric_build::BuildGeneration generation;
    std::shared_ptr<lyric_build::BuildState> buildState;
    std::shared_ptr<tempo_tracing::TraceSpan> span;
    std::filesystem::path testerDirectory;

    std::filesystem::path writeNamedFile(
        const std::filesystem::path &baseDir,
        const std::filesystem::path &path,
        std::string_view content);

    std::filesystem::path writeTempFile(
        const std::filesystem::path &baseDir,
        const std::filesystem::path &templatePath,
        std::string_view content);

    void SetUp() override;
    virtual void configure();

    lyric_build::TempDirectory *tempDirectory();

private:
    std::shared_ptr<tempo_tracing::TraceRecorder> m_recorder;
    std::shared_ptr<lyric_build::LocalFilesystem> m_vfs;
    std::unique_ptr<lyric_build::TempDirectory> m_tmp;
};

#endif // ZURI_BUILD_BASE_TASK_FIXTURE_H