#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <tempo_test/tempo_test.h>
#include <zuri_distributor/dependency_set.h>
#include <zuri_test/zuri_tester.h>

#include "zuri_distributor/abstract_package_resolver.h"
#include "zuri_distributor/dependency_selector.h"
#include "zuri_distributor/static_package_resolver.h"

class DependencySelector : public ::testing::Test {
protected:
    std::shared_ptr<zuri_distributor::AbstractPackageResolver> resolver;

    zuri_packager::PackageSpecifier a1_0_0 = {"a", "foo", 1, 0, 0};
    zuri_packager::PackageSpecifier b1_1_0 = {"b", "foo", 1, 1, 0};
    zuri_packager::PackageSpecifier b1_2_0 = {"b", "foo", 1, 2, 0};
    zuri_packager::PackageSpecifier c1_1_0 = {"c", "foo", 1, 1, 0};
    zuri_packager::PackageSpecifier c1_2_0 = {"c", "foo", 1, 2, 0};
    zuri_packager::PackageSpecifier c1_3_0 = {"c", "foo", 1, 3, 0};
    zuri_packager::PackageSpecifier d1_1_0 = {"d", "foo", 1, 1, 0};
    zuri_packager::PackageSpecifier d1_2_0 = {"d", "foo", 1, 2, 0};
    zuri_packager::PackageSpecifier d1_3_0 = {"d", "foo", 1, 3, 0};
    zuri_packager::PackageSpecifier d1_4_0 = {"d", "foo", 1, 4, 0};
    zuri_packager::PackageSpecifier e1_1_0 = {"e", "foo", 1, 1, 0};
    zuri_packager::PackageSpecifier e1_2_0 = {"e", "foo", 1, 2, 0};
    zuri_packager::PackageSpecifier e1_3_0 = {"e", "foo", 1, 3, 0};
    zuri_packager::PackageSpecifier f1_1_0 = {"f", "foo", 1, 1, 0};
    zuri_packager::PackageSpecifier g1_1_0 = {"g", "foo", 1, 1, 0};

    void SetUp() override {
        absl::btree_map<zuri_packager::PackageSpecifier,zuri_distributor::PackageVersionDescriptor> versions;
        /*
         *            <A1.0>
         *               |`-----------------.
         *   <B1.1>   <B1.2>      <C1.1>   <C1.2>   <C1.3>
         *      |        `------.          .--'        |
         *   <D1.1>   <D1.2>   <D1.3>   <D1.4>      <F1.1>
         *      `------. |        | .----'             |
         *            <E1.1>   <E1.2>   <E1.3>      <G1.1>
         */
        versions[a1_0_0] = {a1_0_0.getPackageId(), a1_0_0.getPackageVersion(),
            {b1_2_0, c1_2_0}};
        versions[b1_1_0] = {b1_1_0.getPackageId(), b1_1_0.getPackageVersion(),
            {d1_1_0}};
        versions[b1_2_0] = {b1_2_0.getPackageId(), b1_2_0.getPackageVersion(),
            {d1_3_0}};
        versions[c1_1_0] = {c1_1_0.getPackageId(), c1_1_0.getPackageVersion(),
            {}};
        versions[c1_2_0] = {c1_2_0.getPackageId(), c1_2_0.getPackageVersion(),
            {d1_4_0}};
        versions[c1_3_0] = {c1_3_0.getPackageId(), c1_3_0.getPackageVersion(),
            {f1_1_0}};
        versions[d1_1_0] = {d1_1_0.getPackageId(), d1_1_0.getPackageVersion(),
            {e1_1_0}};
        versions[d1_2_0] = {d1_2_0.getPackageId(), d1_2_0.getPackageVersion(),
            {e1_1_0}};
        versions[d1_3_0] = {d1_3_0.getPackageId(), d1_3_0.getPackageVersion(),
            {e1_2_0}};
        versions[d1_4_0] = {d1_4_0.getPackageId(), d1_4_0.getPackageVersion(),
            {e1_2_0}};
        versions[f1_1_0] = {f1_1_0.getPackageId(), f1_1_0.getPackageVersion(),
            {g1_1_0}};
        versions[e1_1_0] = {e1_1_0.getPackageId(), e1_1_0.getPackageVersion(),
            {}};
        versions[e1_2_0] = {e1_2_0.getPackageId(), e1_2_0.getPackageVersion(),
            {}};
        versions[e1_3_0] = {e1_3_0.getPackageId(), e1_3_0.getPackageVersion(),
            {}};
        versions[g1_1_0] = {g1_1_0.getPackageId(), g1_1_0.getPackageVersion(),
            {}};
        TU_ASSIGN_OR_RAISE (resolver, zuri_distributor::StaticPackageResolver::create(versions));
    }
};

TEST_F(DependencySelector, AddSingleDirectDependency)
{
    zuri_distributor::DependencySelector selector(resolver);

    ASSERT_THAT (selector.addDirectDependency(g1_1_0), tempo_test::IsOk());

    auto dependencyOrderResult = selector.calculateDependencyOrder();
    ASSERT_THAT (dependencyOrderResult, tempo_test::IsResult());
    auto dependencyOrder = dependencyOrderResult.getResult();

    ASSERT_EQ (1, dependencyOrder.size());
    ASSERT_EQ (g1_1_0, dependencyOrder.at(0));
}

TEST_F(DependencySelector, AddSingleDirectDependencyWithTransitiveDependencies)
{
    zuri_distributor::DependencySelector selector(resolver);

    ASSERT_THAT (selector.addDirectDependency(c1_3_0), tempo_test::IsOk());

    auto dependencyOrderResult = selector.calculateDependencyOrder();
    ASSERT_THAT (dependencyOrderResult, tempo_test::IsResult());
    auto dependencyOrder = dependencyOrderResult.getResult();

    ASSERT_EQ (3, dependencyOrder.size());
    ASSERT_EQ (g1_1_0, dependencyOrder.at(0));
    ASSERT_EQ (f1_1_0, dependencyOrder.at(1));
    ASSERT_EQ (c1_3_0, dependencyOrder.at(2));
}

TEST_F(DependencySelector, AddMultipleDirectDependencies)
{
    zuri_distributor::DependencySelector selector(resolver);

    ASSERT_THAT (selector.addDirectDependency(c1_1_0), tempo_test::IsOk());
    ASSERT_THAT (selector.addDirectDependency(e1_2_0), tempo_test::IsOk());
    ASSERT_THAT (selector.addDirectDependency(g1_1_0), tempo_test::IsOk());

    auto dependencyOrderResult = selector.calculateDependencyOrder();
    ASSERT_THAT (dependencyOrderResult, tempo_test::IsResult());
    auto dependencyOrder = dependencyOrderResult.getResult();

    ASSERT_EQ (3, dependencyOrder.size());
    ASSERT_THAT (dependencyOrder, testing::UnorderedElementsAre(c1_1_0, e1_2_0, g1_1_0));
}

TEST_F(DependencySelector, AddMultipleDirectDependenciesWithTransitiveDependencies)
{
    zuri_distributor::DependencySelector selector(resolver);

    ASSERT_THAT (selector.addDirectDependency(b1_1_0), tempo_test::IsOk());
    ASSERT_THAT (selector.addDirectDependency(c1_3_0), tempo_test::IsOk());

    auto dependencyOrderResult = selector.calculateDependencyOrder();
    ASSERT_THAT (dependencyOrderResult, tempo_test::IsResult());
    auto dependencyOrder = dependencyOrderResult.getResult();

    ASSERT_EQ (6, dependencyOrder.size());
    ASSERT_THAT (dependencyOrder, testing::UnorderedElementsAre(
        b1_1_0, d1_1_0, e1_1_0, c1_3_0, f1_1_0, g1_1_0));
}

TEST_F(DependencySelector, AddMultipleDirectDependenciesWithMinimumSelection)
{
    zuri_distributor::DependencySelector selector(resolver);

    ASSERT_THAT (selector.addDirectDependency(b1_1_0), tempo_test::IsOk());
    ASSERT_THAT (selector.addDirectDependency(c1_2_0), tempo_test::IsOk());

    auto dependencyOrderResult = selector.calculateDependencyOrder();
    ASSERT_THAT (dependencyOrderResult, tempo_test::IsResult());
    auto dependencyOrder = dependencyOrderResult.getResult();

    ASSERT_EQ (4, dependencyOrder.size());
    ASSERT_THAT (dependencyOrder, testing::UnorderedElementsAre(
        b1_1_0, c1_2_0, d1_4_0, e1_2_0));
}
