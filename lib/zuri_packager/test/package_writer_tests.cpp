#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <tempo_test/result_matchers.h>
#include <tempo_test/status_matchers.h>
#include <tempo_utils/file_utilities.h>
#include <tempo_utils/memory_bytes.h>

#include <zuri_packager/package_writer.h>

class PackageWriter : public ::testing::Test {
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

TEST_F(PackageWriter, WriteEmptyPackage)
{
    auto specifier = zuri_packager::PackageSpecifier::fromString("foo-1.0.0@foocorp");
    zuri_packager::PackageWriter writer(specifier);
    ASSERT_THAT (writer.configure(), tempo_test::IsOk());

    auto writePackageResult = writer.writePackage();
    ASSERT_THAT (writePackageResult, tempo_test::IsResult());
    packagePath = writePackageResult.getResult();
}

TEST_F(PackageWriter, AddFileAndWritePackage)
{
    auto specifier = zuri_packager::PackageSpecifier::fromString("foo-1.0.0@foocorp");
    zuri_packager::PackageWriter writer(specifier);
    ASSERT_THAT (writer.configure(), tempo_test::IsOk());

    auto path = tempo_utils::UrlPath::fromString("/file.txt");
    auto content = tempo_utils::MemoryBytes::copy("hello, world!");
    auto putFileResult = writer.putFile(path, content);
    ASSERT_THAT (putFileResult, tempo_test::IsResult());

    auto writePackageResult = writer.writePackage();
    ASSERT_THAT (writePackageResult, tempo_test::IsResult());
    packagePath = writePackageResult.getResult();
}

TEST_F(PackageWriter, AddFileInDirectoryAndWritePackage)
{
    auto specifier = zuri_packager::PackageSpecifier::fromString("foo-1.0.0@foocorp");
    zuri_packager::PackageWriter writer(specifier);
    ASSERT_THAT (writer.configure(), tempo_test::IsOk());

    auto dirPath = tempo_utils::UrlPath::fromString("/dir");
    auto makeDirectoryResult = writer.makeDirectory(dirPath);
    ASSERT_THAT (makeDirectoryResult, tempo_test::IsResult());

    auto dir = makeDirectoryResult.getResult();
    auto content = tempo_utils::MemoryBytes::copy("hello, world!");
    auto putFileResult = writer.putFile(dir, "file.txt", content);
    ASSERT_THAT (putFileResult, tempo_test::IsResult());

    auto writePackageResult = writer.writePackage();
    ASSERT_THAT (writePackageResult, tempo_test::IsResult());
    packagePath = writePackageResult.getResult();
}
