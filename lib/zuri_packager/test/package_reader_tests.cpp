#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <tempo_test/result_matchers.h>
#include <tempo_test/status_matchers.h>
#include <tempo_utils/file_reader.h>
#include <tempo_utils/file_utilities.h>
#include <tempo_utils/memory_bytes.h>
#include <tempo_utils/tempfile_maker.h>
#include <zuri_packager/package_reader.h>
#include <zuri_packager/package_writer.h>

class PackageReader : public ::testing::Test {
protected:
    std::string packageName;
    std::filesystem::path packagePath;
    void SetUp() override {
        packageName = tempo_utils::generate_name("XXXXXXXX");
    }
    void TearDown() override {
        if (std::filesystem::exists(packagePath)) {
            TU_ASSERT (std::filesystem::remove(packagePath));
            packagePath.clear();
        }
    }
};

TEST_F(PackageReader, OpenPackageFile)
{
    auto specifier = zuri_packager::PackageSpecifier::fromString("foo-1.0.0@foocorp");
    zuri_packager::PackageWriter writer(specifier);
    ASSERT_THAT (writer.configure(), tempo_test::IsOk());

    auto path = tempo_utils::UrlPath::fromString("/file.txt");
    auto content = tempo_utils::MemoryBytes::copy("hello, world!");
    writer.putFile(path, content);
    auto writePackageResult = writer.writePackage();
    ASSERT_THAT (writePackageResult, tempo_test::IsResult());

    packagePath = writePackageResult.getResult();
    auto openReaderResult = zuri_packager::PackageReader::open(packagePath);
    ASSERT_THAT (openReaderResult, tempo_test::IsResult());

    auto reader = openReaderResult.getResult();
    ASSERT_TRUE (reader->isValid());
    auto readContentsResult = reader->readFileContents(path);
    ASSERT_THAT (readContentsResult, tempo_test::IsResult());
    auto slice = readContentsResult.getResult();
    ASSERT_EQ (13, slice.getSize());
    ASSERT_FALSE (slice.isEmpty());
    std::string contents((const char *) slice.getData(), slice.getSize());
    ASSERT_EQ ("hello, world!", contents);
}

TEST_F(PackageReader, ReadPackageBytes)
{
    auto specifier = zuri_packager::PackageSpecifier::fromString("foo-1.0.0@foocorp");
    zuri_packager::PackageWriter writer(specifier);
    ASSERT_THAT (writer.configure(), tempo_test::IsOk());

    auto path = tempo_utils::UrlPath::fromString("/file.txt");
    auto content = tempo_utils::MemoryBytes::copy("hello, world!");
    writer.putFile(path, content);
    auto writePackageResult = writer.writePackage();
    ASSERT_THAT (writePackageResult, tempo_test::IsResult());

    packagePath = writePackageResult.getResult();

    tempo_utils::FileReader fileReader(packagePath);
    ASSERT_THAT (fileReader.getStatus(), tempo_test::IsOk());

    auto createReaderResult = zuri_packager::PackageReader::create(fileReader.getBytes());
    ASSERT_THAT (createReaderResult, tempo_test::IsResult());

    auto reader = createReaderResult.getResult();
    ASSERT_TRUE (reader->isValid());
    auto readContentsResult = reader->readFileContents(path);
    ASSERT_THAT (readContentsResult, tempo_test::IsResult());
    auto slice = readContentsResult.getResult();
    ASSERT_EQ (13, slice.getSize());
    ASSERT_FALSE (slice.isEmpty());
    std::string contents((const char *) slice.getData(), slice.getSize());
    ASSERT_EQ ("hello, world!", contents);
}
