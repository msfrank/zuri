#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <zuri_packager/package_requirement.h>

#include "zuri_packager/package_dependency.h"

TEST(PackageDependency, SatisfiesSingleRequirement) {
    auto req = zuri_packager::VersionRequirement::create(
        zuri_packager::PackageSpecifier::fromString("foo-1.0.1@foocorp"),
        zuri_packager::VersionComparison::Equal);

    zuri_packager::PackageDependency dep("foo", "foocorp", {req});
    ASSERT_TRUE (dep.satisfiedBy(zuri_packager::PackageSpecifier::fromString("foo-1.0.1@foocorp")));
}

TEST(PackageDependency, SatisfiesMultipleRequirements) {
    auto req1 = zuri_packager::VersionRequirement::create(
        zuri_packager::PackageSpecifier::fromString("foo-1.0.0@foocorp"),
        zuri_packager::VersionComparison::GreaterOrEqual);
    auto req2 = zuri_packager::VersionRequirement::create(
        zuri_packager::PackageSpecifier::fromString("foo-2.0.0@foocorp"),
        zuri_packager::VersionComparison::LesserThan);

    zuri_packager::PackageDependency dep("foo", "foocorp", {req1, req2});
    ASSERT_TRUE (dep.satisfiedBy(zuri_packager::PackageSpecifier::fromString("foo-1.0.1@foocorp")));
}
