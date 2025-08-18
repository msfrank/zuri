#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <tempo_test/tempo_test.h>
#include <zuri_distributor/http_package_resolver.h>
#include <zuri_test/zuri_tester.h>

TEST(HttpPackageResolver, GetRepository)
{
    std::shared_ptr<zuri_distributor::HttpPackageResolver> resolver;
    TU_ASSIGN_OR_RAISE (resolver, zuri_distributor::HttpPackageResolver::create());
    auto getRepositoryResult = resolver->getRepository("msfrank.github.io");
    ASSERT_THAT (getRepositoryResult, tempo_test::IsResult());
}

TEST(HttpPackageResolver, GetCollection)
{
    std::shared_ptr<zuri_distributor::HttpPackageResolver> resolver;
    TU_ASSIGN_OR_RAISE (resolver, zuri_distributor::HttpPackageResolver::create());
    auto getCollectionResult = resolver->getCollection(zuri_packager::PackageId("test", "msfrank.github.io"));
    ASSERT_THAT (getCollectionResult, tempo_test::IsResult());
}

TEST(HttpPackageResolver, GetPackage)
{
    std::shared_ptr<zuri_distributor::HttpPackageResolver> resolver;
    TU_ASSIGN_OR_RAISE (resolver, zuri_distributor::HttpPackageResolver::create());
    auto getPackageResult = resolver->getPackage(
        zuri_packager::PackageId("test", "msfrank.github.io"),
        zuri_packager::PackageVersion(0, 0, 1));
    ASSERT_THAT (getPackageResult, tempo_test::IsResult());
}

