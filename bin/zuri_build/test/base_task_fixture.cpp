
#include <lyric_build/memory_cache.h>
#include <lyric_runtime/static_loader.h>
#include <tempo_utils/file_writer.h>
#include <tempo_utils/tempdir_maker.h>
#include <tempo_utils/tempfile_maker.h>

#include "base_task_fixture.h"

void
BaseTaskFixture::SetUp()
{
    generation = lyric_build::BuildGeneration::create();
    m_recorder = tempo_tracing::TraceRecorder::create();
    span = m_recorder->makeSpan();

    tempo_utils::TempdirMaker tempdirMaker(std::filesystem::current_path(), "tester.XXXXXXXX");
    TU_RAISE_IF_NOT_OK (tempdirMaker.getStatus());
    testerDirectory = tempdirMaker.getTempdir();

    TU_LOG_INFO << "created tester directory " << testerDirectory;

    TU_ASSIGN_OR_RAISE (m_vfs, lyric_build::LocalFilesystem::create(testerDirectory));

    auto buildgen = lyric_build::BuildGeneration::create();
    auto cache = std::make_shared<lyric_build::MemoryCache>();
    auto bootstrapLoader = std::make_shared<lyric_bootstrap::BootstrapLoader>();
    auto sharedModuleCache = lyric_importer::ModuleCache::create(bootstrapLoader);

    buildState = lyric_build::BuildState::create(buildgen,
        std::static_pointer_cast<lyric_build::AbstractArtifactCache>(cache),
        bootstrapLoader,
        std::shared_ptr<lyric_runtime::AbstractLoader>{},
        sharedModuleCache,
        std::make_shared<lyric_importer::ShortcutResolver>(),
        m_vfs,
        testerDirectory);

    configure();
}

std::filesystem::path
BaseTaskFixture::writeNamedFile(
    const std::filesystem::path &baseDir,
    const std::filesystem::path &path,
    std::string_view content)
{
    auto relativePath = baseDir / path;
    auto absolutePath = testerDirectory / relativePath;
    auto parentPath = absolutePath.parent_path();
    std::error_code ec;
    if (!std::filesystem::create_directories(parentPath, ec) && ec) {
        auto status = tempo_utils::GenericStatus::forCondition(tempo_utils::GenericCondition::kInternalViolation,
            "failed to create directory {}: {}", parentPath.string(), ec.message());
        throw tempo_utils::StatusException(status);
    }

    tempo_utils::FileWriter writer(absolutePath.string(), content, tempo_utils::FileWriterMode::CREATE_OR_OVERWRITE);
    if (!writer.isValid()) {
        auto status = tempo_utils::GenericStatus::forCondition(tempo_utils::GenericCondition::kInternalViolation,
            "failed to write file {}", absolutePath.string());
        throw tempo_utils::StatusException(status);
    }

    return relativePath;
}

std::filesystem::path
BaseTaskFixture::writeTempFile(
    const std::filesystem::path &baseDir,
    const std::filesystem::path &templatePath,
    std::string_view content)
{
    auto absolutePath = testerDirectory / baseDir / templatePath;
    auto parentPath = absolutePath.parent_path();
    std::error_code ec;
    if (!std::filesystem::create_directories(parentPath, ec) && ec) {
        auto status = tempo_utils::GenericStatus::forCondition(tempo_utils::GenericCondition::kInternalViolation,
            "failed to create directory {}: {}", parentPath.string(), ec.message());
        throw tempo_utils::StatusException(status);
    }

    tempo_utils::TempfileMaker tempfileMaker(absolutePath.parent_path().string(),
        absolutePath.filename().string(), content);
    if (!tempfileMaker.isValid()) {
        auto status = tempo_utils::GenericStatus::forCondition(tempo_utils::GenericCondition::kInternalViolation,
            "failed to create temp file {}", absolutePath.string());
        throw tempo_utils::StatusException(status);
    }

    return std::filesystem::relative(tempfileMaker.getTempfile(), testerDirectory);
}

void
BaseTaskFixture::configure()
{
    taskSettings = lyric_build::TaskSettings();
}


lyric_build::TempDirectory *
BaseTaskFixture::tempDirectory()
{
    if (m_tmp == nullptr) {
        m_tmp = std::make_unique<lyric_build::TempDirectory>(testerDirectory, "tmp");
    }
    return m_tmp.get();

}
