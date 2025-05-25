#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <tempo_config/config_builder.h>
#include <tempo_config/config_serde.h>

#include <tempo_test/result_matchers.h>
#include <tempo_test/status_matchers.h>
#include <tempo_utils/file_utilities.h>
#include <tempo_utils/memory_bytes.h>
#include <tempo_utils/tempdir_maker.h>

#include <zuri_packager/package_extractor.h>
#include <zuri_packager/package_writer.h>

class PackageExtractor : public ::testing::Test {
protected:
    std::filesystem::path testerRoot;
    void SetUp() override {
        tempo_utils::TempdirMaker installMaker(std::filesystem::current_path(), "tester.XXXXXXXX");
        TU_RAISE_IF_NOT_OK (installMaker.getStatus());
        testerRoot = installMaker.getTempdir();
    }
    void TearDown() override {
        if (std::filesystem::exists(testerRoot)) {
            TU_ASSERT (std::filesystem::remove_all(testerRoot));
            testerRoot.clear();
        }
    }
};

TEST_F(PackageExtractor, ExtractFailsWhenMissingPackageConfig)
{
    auto specifier = zuri_packager::PackageSpecifier::fromString("foo-1.0.0@foocorp");
    zuri_packager::PackageWriterOptions writerOptions;
    writerOptions.installRoot = testerRoot;
    zuri_packager::PackageWriter writer(specifier, writerOptions);
    ASSERT_THAT (writer.configure(), tempo_test::IsOk());

    auto writePackageResult = writer.writePackage();
    ASSERT_THAT (writePackageResult, tempo_test::IsResult());
    auto packagePath = writePackageResult.getResult();

    zuri_packager::PackageExtractorOptions extractorOptions;
    extractorOptions.workingRoot = testerRoot;
    extractorOptions.distributionRoot = testerRoot;
    zuri_packager::PackageExtractor extractor(packagePath, extractorOptions);
    ASSERT_THAT (extractor.configure(), tempo_test::IsOk());

    auto extractPackageResult = extractor.extractPackage();
    ASSERT_THAT (extractPackageResult, tempo_test::IsStatus());
}

TEST_F(PackageExtractor, ExtractEmptyPackage)
{
    auto specifier = zuri_packager::PackageSpecifier::fromString("foo-1.0.0@foocorp");
    zuri_packager::PackageWriterOptions writerOptions;
    writerOptions.installRoot = testerRoot;
    zuri_packager::PackageWriter writer(specifier, writerOptions);
    ASSERT_THAT (writer.configure(), tempo_test::IsOk());

    auto path = tempo_utils::UrlPath::fromString("/package.config");
    auto content = tempo_utils::MemoryBytes::copy(R"(
    {
        "name": "foo",
        "version": "1.0.0",
        "domain": "foocorp"
    }
    )");
    auto putFileResult = writer.putFile(path, content);
    ASSERT_THAT (putFileResult, tempo_test::IsResult());

    auto writePackageResult = writer.writePackage();
    ASSERT_THAT (writePackageResult, tempo_test::IsResult());
    auto packagePath = writePackageResult.getResult();

    zuri_packager::PackageExtractorOptions extractorOptions;
    extractorOptions.workingRoot = testerRoot;
    extractorOptions.distributionRoot = testerRoot;
    zuri_packager::PackageExtractor extractor(packagePath, extractorOptions);
    ASSERT_THAT (extractor.configure(), tempo_test::IsOk());

    auto extractPackageResult = extractor.extractPackage();
    ASSERT_THAT (extractPackageResult, tempo_test::IsResult());
    auto extractPath = extractPackageResult.getResult();
}
