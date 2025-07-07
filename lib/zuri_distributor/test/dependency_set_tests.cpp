#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <tempo_test/tempo_test.h>
#include <zuri_distributor/dependency_set.h>
#include <zuri_test/zuri_tester.h>

TEST(DependencySet, AddSingleExactDependency)
{
    zuri_packager::PackageId rootId("foo", "zuri.dev");
    zuri_distributor::DependencySet dependencySet(rootId);

    zuri_packager::PackageId barId("bar", "zuri.dev");
    zuri_packager::PackageDependency bar(barId.getName(), barId.getDomain(),
        {zuri_packager::ExactVersionRequirement::create(
            zuri_packager::PackageVersion(1, 0, 1))});
    ASSERT_THAT (dependencySet.addDependency(bar), tempo_test::IsOk());

    auto resolutionOrderResult = dependencySet.calculateResolutionOrder();
    ASSERT_THAT (resolutionOrderResult, tempo_test::IsResult());
    auto resolutionOrder = resolutionOrderResult.getResult();

    ASSERT_EQ (1, resolutionOrder.size());
    auto res1 = resolutionOrder.at(0);
    ASSERT_EQ (barId, res1.packageId);

    ASSERT_EQ (1, res1.validIntervals.size());
    auto interval = res1.validIntervals.at(0);
    ASSERT_EQ (zuri_packager::PackageVersion(1, 0, 1), interval.closedLowerBound);
    ASSERT_EQ (zuri_packager::PackageVersion(1, 0, 2), interval.openUpperBound);
}

TEST(DependencySet, AddMultipleExactDependencies)
{
    zuri_packager::PackageId rootId("foo", "zuri.dev");
    zuri_distributor::DependencySet dependencySet(rootId);

    zuri_packager::PackageId barId("bar", "zuri.dev");
    zuri_packager::PackageDependency bar(barId.getName(), barId.getDomain(),
        {zuri_packager::ExactVersionRequirement::create(
            zuri_packager::PackageVersion(1, 0, 1))});
    ASSERT_THAT (dependencySet.addDependency(bar), tempo_test::IsOk());

    zuri_packager::PackageId bazId("baz", "zuri.dev");
    zuri_packager::PackageDependency baz(bazId.getName(), bazId.getDomain(),
        {zuri_packager::ExactVersionRequirement::create(
            zuri_packager::PackageVersion(2, 0, 1))});
    ASSERT_THAT (dependencySet.addDependency(baz), tempo_test::IsOk());

    zuri_packager::PackageId quxId("qux", "zuri.dev");
    zuri_packager::PackageDependency qux(quxId.getName(), quxId.getDomain(),
        {zuri_packager::ExactVersionRequirement::create(
            zuri_packager::PackageVersion(3, 0, 1))});
    ASSERT_THAT (dependencySet.addDependency(qux), tempo_test::IsOk());

    auto resolutionOrderResult = dependencySet.calculateResolutionOrder();
    ASSERT_THAT (resolutionOrderResult, tempo_test::IsResult());
    auto resolutionOrder = resolutionOrderResult.getResult();

    ASSERT_EQ (3, resolutionOrder.size());
    absl::flat_hash_map<zuri_packager::PackageId,int> idIndex;
    for (int i = 0; i < resolutionOrder.size(); ++i) {
        const auto &resolved = resolutionOrder.at(i);
        idIndex[resolved.packageId] = i;
    }

    const auto &barPackage = resolutionOrder.at(idIndex[barId]);
    ASSERT_EQ (1, barPackage.validIntervals.size());
    auto barInterval1 = barPackage.validIntervals.at(0);
    ASSERT_EQ (zuri_packager::PackageVersion(1, 0, 1), barInterval1.closedLowerBound);
    ASSERT_EQ (zuri_packager::PackageVersion(1, 0, 2), barInterval1.openUpperBound);

    const auto &bazPackage = resolutionOrder.at(idIndex[bazId]);
    ASSERT_EQ (1, bazPackage.validIntervals.size());
    auto bazInterval1 = bazPackage.validIntervals.at(0);
    ASSERT_EQ (zuri_packager::PackageVersion(2, 0, 1), bazInterval1.closedLowerBound);
    ASSERT_EQ (zuri_packager::PackageVersion(2, 0, 2), bazInterval1.openUpperBound);

    const auto &quxPackage = resolutionOrder.at(idIndex[quxId]);
    ASSERT_EQ (1, quxPackage.validIntervals.size());
    auto quxInterval1 = quxPackage.validIntervals.at(0);
    ASSERT_EQ (zuri_packager::PackageVersion(3, 0, 1), quxInterval1.closedLowerBound);
    ASSERT_EQ (zuri_packager::PackageVersion(3, 0, 2), quxInterval1.openUpperBound);
}

TEST(DependencySet, AddDependencyWithMultipleTargets)
{
    zuri_packager::PackageId rootId("foo", "zuri.dev");
    zuri_distributor::DependencySet dependencySet(rootId);

    zuri_packager::PackageId barId("bar", "zuri.dev");
    zuri_packager::PackageDependency bar(barId.getName(), barId.getDomain(),
        {zuri_packager::ExactVersionRequirement::create(
            zuri_packager::PackageVersion(1, 0, 1))});
    ASSERT_THAT (dependencySet.addDependency(bar), tempo_test::IsOk());

    zuri_packager::PackageId bazId("baz", "zuri.dev");
    zuri_packager::PackageDependency baz(bazId.getName(), bazId.getDomain(),
        {zuri_packager::ExactVersionRequirement::create(
            zuri_packager::PackageVersion(2, 0, 1))});
    ASSERT_THAT (dependencySet.addDependency(baz), tempo_test::IsOk());

    zuri_packager::PackageId quxId("qux", "zuri.dev");
    zuri_packager::PackageDependency qux(quxId.getName(), quxId.getDomain(),
        {zuri_packager::ExactVersionRequirement::create(
            zuri_packager::PackageVersion(3, 0, 1))});
    ASSERT_THAT (dependencySet.addDependency(barId, qux), tempo_test::IsOk());
    ASSERT_THAT (dependencySet.addDependency(bazId, qux), tempo_test::IsOk());

    auto resolutionOrderResult = dependencySet.calculateResolutionOrder();
    ASSERT_THAT (resolutionOrderResult, tempo_test::IsResult());
    auto resolutionOrder = resolutionOrderResult.getResult();

    ASSERT_EQ (3, resolutionOrder.size());
    absl::flat_hash_map<zuri_packager::PackageId,int> idIndex;
    for (int i = 0; i < resolutionOrder.size(); ++i) {
        const auto &resolved = resolutionOrder.at(i);
        idIndex[resolved.packageId] = i;
    }

    auto res1 = resolutionOrder.at(0);
    ASSERT_EQ (quxId, res1.packageId);

    ASSERT_EQ (1, res1.validIntervals.size());
    auto interval = res1.validIntervals.at(0);
    ASSERT_EQ (zuri_packager::PackageVersion(3, 0, 1), interval.closedLowerBound);
    ASSERT_EQ (zuri_packager::PackageVersion(3, 0, 2), interval.openUpperBound);

    std::vector res1And2{resolutionOrder[1].packageId, resolutionOrder[2].packageId};
    ASSERT_THAT (res1And2, ::testing::UnorderedElementsAre(barId, bazId));
}

TEST(DependencySet, AddDependencyWithMultipleTargetsOverlappingIntervals)
{
    zuri_packager::PackageId rootId("foo", "zuri.dev");
    zuri_distributor::DependencySet dependencySet(rootId);

    zuri_packager::PackageId barId("bar", "zuri.dev");
    zuri_packager::PackageDependency bar(barId.getName(), barId.getDomain(),
        {zuri_packager::ExactVersionRequirement::create(
            zuri_packager::PackageVersion(1, 0, 1))});
    ASSERT_THAT (dependencySet.addDependency(bar), tempo_test::IsOk());

    zuri_packager::PackageId bazId("baz", "zuri.dev");
    zuri_packager::PackageDependency baz(bazId.getName(), bazId.getDomain(),
        {zuri_packager::ExactVersionRequirement::create(
            zuri_packager::PackageVersion(2, 0, 1))});
    ASSERT_THAT (dependencySet.addDependency(baz), tempo_test::IsOk());

    zuri_packager::PackageId quxId("qux", "zuri.dev");

    zuri_packager::PackageDependency barQux(quxId.getName(), quxId.getDomain(),
        {zuri_packager::VersionRangeRequirement::create(
            zuri_packager::PackageVersion(3, 0, 0),
            zuri_packager::PackageVersion(4, 0, 0))});
    ASSERT_THAT (dependencySet.addDependency(barId, barQux), tempo_test::IsOk());

    zuri_packager::PackageDependency bazQux(quxId.getName(), quxId.getDomain(),
        {zuri_packager::VersionRangeRequirement::create(
            zuri_packager::PackageVersion(3, 5, 0),
            zuri_packager::PackageVersion(4, 5, 0))});
    ASSERT_THAT (dependencySet.addDependency(bazId, bazQux), tempo_test::IsOk());

    auto resolutionOrderResult = dependencySet.calculateResolutionOrder();
    ASSERT_THAT (resolutionOrderResult, tempo_test::IsResult());
    auto resolutionOrder = resolutionOrderResult.getResult();
    ASSERT_EQ (3, resolutionOrder.size());

    auto res1 = resolutionOrder.at(0);
    ASSERT_EQ (quxId, res1.packageId);

    ASSERT_EQ (1, res1.validIntervals.size());
    auto interval = res1.validIntervals.at(0);
    ASSERT_EQ (zuri_packager::PackageVersion(3, 5, 0), interval.closedLowerBound);
    ASSERT_EQ (zuri_packager::PackageVersion(4, 0, 1), interval.openUpperBound);
}
