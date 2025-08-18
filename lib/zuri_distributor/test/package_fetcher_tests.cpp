#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <tempo_test/tempo_test.h>
#include <tempo_utils/directory_maker.h>
#include <tempo_utils/tempdir_maker.h>
#include <zuri_distributor/package_fetcher.h>
#include <zuri_packager/package_writer.h>
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

        zuri_packager::PackageWriterOptions options;
        options.installRoot = fetchRoot;

        // foo-1.0.1@foocorp
        fooSpecifier = zuri_packager::PackageSpecifier("foo", "foocorp", 1, 0, 1);
        zuri_packager::PackageWriter fooWriter(fooSpecifier, options);
        fooWriter.configure();
        std::filesystem::path fooPath;
        TU_ASSIGN_OR_RAISE (fooPath, fooWriter.writePackage());
        fooUrl = tempo_utils::Url::fromFilesystemPath(fooPath);

        // bar-1.0.2@foocorp
        barSpecifier = zuri_packager::PackageSpecifier("bar", "foocorp", 1, 0, 2);
        zuri_packager::PackageWriter barWriter(barSpecifier, options);
        barWriter.configure();
        std::filesystem::path barPath;
        TU_ASSIGN_OR_RAISE (barPath, barWriter.writePackage());
        barUrl = tempo_utils::Url::fromFilesystemPath(barPath);

        // baz-1.0.3@foocorp
        bazSpecifier = zuri_packager::PackageSpecifier("baz", "foocorp", 1, 0, 3);
        zuri_packager::PackageWriter bazWriter(bazSpecifier, options);
        bazWriter.configure();
        std::filesystem::path bazPath;
        TU_ASSIGN_OR_RAISE (bazPath, bazWriter.writePackage());
        bazUrl = tempo_utils::Url::fromFilesystemPath(bazPath);

        // qux-1.0.4@foocorp
        quxSpecifier = zuri_packager::PackageSpecifier("qux", "foocorp", 1, 0, 4);
        zuri_packager::PackageWriter quxWriter(quxSpecifier, options);
        quxWriter.configure();
        std::filesystem::path quxPath;
        TU_ASSIGN_OR_RAISE (quxPath, quxWriter.writePackage());
        quxUrl = tempo_utils::Url::fromFilesystemPath(quxPath);
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
    auto id = fooSpecifier.toString();
    ASSERT_THAT (fetcher.requestFile(fooUrl, id), tempo_test::IsOk());
    ASSERT_THAT (fetcher.fetchFiles(), tempo_test::IsOk());

    ASSERT_EQ (1, fetcher.numResults());
    ASSERT_TRUE (fetcher.hasResult(id));

    auto result = fetcher.getResult(id);
    ASSERT_THAT (result.status, tempo_test::IsOk());
    ASSERT_TRUE (std::filesystem::is_regular_file(result.path));

    auto openPackageResult = zuri_packager::PackageReader::open(result.path);
    ASSERT_THAT (openPackageResult, tempo_test::IsResult());
    auto reader1 = openPackageResult.getResult();

    auto readPackageSpecifier = reader1->readPackageSpecifier();
    ASSERT_THAT (readPackageSpecifier, tempo_test::IsResult());
    ASSERT_EQ (fooSpecifier, readPackageSpecifier.getResult());
}

TEST_F (PackageFetcher, FetchMultiplePackagesFromFileUrl) {
    zuri_distributor::PackageFetcherOptions options;
    options.downloadRoot = downloadDir->getAbsolutePath();

    zuri_distributor::PackageFetcher fetcher(options);
    ASSERT_THAT (fetcher.configure(), tempo_test::IsOk());
    auto fooId = fooSpecifier.toString();
    ASSERT_THAT (fetcher.requestFile(fooUrl, fooId), tempo_test::IsOk());
    auto barId = barSpecifier.toString();
    ASSERT_THAT (fetcher.requestFile(barUrl, barId), tempo_test::IsOk());
    auto bazId = bazSpecifier.toString();
    ASSERT_THAT (fetcher.requestFile(bazUrl, bazId), tempo_test::IsOk());
    ASSERT_THAT (fetcher.fetchFiles(), tempo_test::IsOk());

    ASSERT_EQ (3, fetcher.numResults());
    ASSERT_TRUE (fetcher.hasResult(fooId));
    ASSERT_TRUE (fetcher.hasResult(barId));
    ASSERT_TRUE (fetcher.hasResult(bazId));

    auto fooResult = fetcher.getResult(fooId);
    auto openFooResult = zuri_packager::PackageReader::open(fooResult.path);
    ASSERT_THAT (openFooResult, tempo_test::IsResult());
    auto readFooSpecifier = openFooResult.getResult()->readPackageSpecifier();
    ASSERT_THAT (readFooSpecifier, tempo_test::IsResult());
    ASSERT_EQ (fooSpecifier, readFooSpecifier.getResult());

    auto barResult = fetcher.getResult(barId);
    auto openBarResult = zuri_packager::PackageReader::open(barResult.path);
    ASSERT_THAT (openBarResult, tempo_test::IsResult());
    auto readBarSpecifier = openBarResult.getResult()->readPackageSpecifier();
    ASSERT_THAT (readBarSpecifier, tempo_test::IsResult());
    ASSERT_EQ (barSpecifier, readBarSpecifier.getResult());

    auto bazResult = fetcher.getResult(bazId);
    auto openBazResult = zuri_packager::PackageReader::open(bazResult.path);
    ASSERT_THAT (openBazResult, tempo_test::IsResult());
    auto readBazSpecifier = openBazResult.getResult()->readPackageSpecifier();
    ASSERT_THAT (readBazSpecifier, tempo_test::IsResult());
    ASSERT_EQ (bazSpecifier, readBazSpecifier.getResult());
}