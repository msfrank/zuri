#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <tempo_test/tempo_test.h>
#include <tempo_utils/directory_maker.h>
#include <tempo_utils/file_reader.h>
#include <tempo_utils/file_writer.h>
#include <tempo_utils/tempdir_maker.h>
#include <zuri_distributor/package_fetcher.h>
#include <zuri_test/zuri_tester.h>

class PackageFetcher : public ::testing::Test {
protected:
    std::unique_ptr<tempo_utils::TempdirMaker> fetchDir;
    std::unique_ptr<tempo_utils::DirectoryMaker> downloadDir;
    zuri_packager::PackageSpecifier fooSpecifier;
    tempo_utils::Url fooUrl;
    zuri_packager::PackageSpecifier barSpecifier;
    tempo_utils::Url barUrl;
    zuri_packager::PackageSpecifier bazSpecifier;
    tempo_utils::Url bazUrl;
    zuri_packager::PackageSpecifier quxSpecifier;
    tempo_utils::Url quxUrl;

    void SetUp() override {
        fetchDir = std::make_unique<tempo_utils::TempdirMaker>(std::filesystem::current_path(), "fetch.XXXXXXXX");
        TU_ASSERT (fetchDir->isValid());
        auto fetchRoot = fetchDir->getTempdir();

        downloadDir = std::make_unique<tempo_utils::DirectoryMaker>(fetchRoot, "downloads");
        TU_ASSERT (downloadDir->isValid());

        // foo-1.0.1@foocorp
        tempo_utils::FileWriter fooWriter(
            fetchRoot / "foopackage.zpk", "foo@foocorp", tempo_utils::FileWriterMode::CREATE_ONLY);
        fooSpecifier = zuri_packager::PackageSpecifier("foo", "foocorp", 1, 0, 1);
        fooUrl = tempo_utils::Url::fromFilesystemPath(fooWriter.getAbsolutePath());

        // bar-1.0.2@foocorp
        tempo_utils::FileWriter barWriter(
            fetchRoot / "barpackage.zpk", "bar@foocorp", tempo_utils::FileWriterMode::CREATE_ONLY);
        barSpecifier = zuri_packager::PackageSpecifier("bar", "foocorp", 1, 0, 2);
        barUrl = tempo_utils::Url::fromFilesystemPath(barWriter.getAbsolutePath());

        // baz-1.0.3@foocorp
        tempo_utils::FileWriter bazWriter(
            fetchRoot / "bazpackage.zpk", "baz@foocorp", tempo_utils::FileWriterMode::CREATE_ONLY);
        bazSpecifier = zuri_packager::PackageSpecifier("baz", "foocorp", 1, 0, 3);
        bazUrl = tempo_utils::Url::fromFilesystemPath(bazWriter.getAbsolutePath());

        // qux-1.0.4@foocorp
        tempo_utils::FileWriter quxWriter(
            fetchRoot / "quxpackage.zpk", "qux@foocorp", tempo_utils::FileWriterMode::CREATE_ONLY);
        quxSpecifier = zuri_packager::PackageSpecifier("qux", "foocorp", 1, 0, 4);
        quxUrl = tempo_utils::Url::fromFilesystemPath(quxWriter.getAbsolutePath());
    }
    void TearDown() override {
        auto fetchRoot = fetchDir->getTempdir();
        std::filesystem::remove_all(fetchRoot);
    }
};

TEST_F (PackageFetcher, FetchPackageFromFileUrl)
{
    zuri_distributor::PackageFetcherOptions options;
    options.downloadRoot = downloadDir->getAbsolutePath();

    zuri_distributor::PackageFetcher fetcher(options);
    ASSERT_THAT (fetcher.configure(), tempo_test::IsOk());
    ASSERT_THAT (fetcher.addPackage(fooSpecifier, fooUrl), tempo_test::IsOk());
    ASSERT_THAT (fetcher.fetchPackages(), tempo_test::IsOk());

    ASSERT_EQ (1, fetcher.numResults());
    ASSERT_TRUE (fetcher.hasResult(fooSpecifier));

    auto result = fetcher.getResult(fooSpecifier);
    ASSERT_THAT (result.status, tempo_test::IsOk());
    ASSERT_TRUE (std::filesystem::is_regular_file(result.path));

    tempo_utils::FileReader reader1(result.path);
    ASSERT_THAT (reader1.getStatus(), tempo_test::IsOk());

    auto span = reader1.getBytes()->getSpan();
    std::string_view content((const char *) span.data(), span.size());
    ASSERT_EQ ("foo@foocorp", content);
}