#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <zuri_packager/package_requirement.h>

TEST(VersionRequirement, SatisfiesEqual)
{
    auto req = zuri_packager::VersionRequirement::create(
        zuri_packager::PackageSpecifier::fromString("foo-1.0.1@foocorp"),
        zuri_packager::VersionComparison::Equal);

    ASSERT_TRUE (req->satisfiedBy(zuri_packager::PackageSpecifier::fromString("foo-1.0.1@foocorp")));
}

TEST(VersionRequirement, EqualNotSatisfied)
{
    auto req = zuri_packager::VersionRequirement::create(
        zuri_packager::PackageSpecifier::fromString("foo-1.0.1@foocorp"),
        zuri_packager::VersionComparison::Equal);

    ASSERT_FALSE (req->satisfiedBy(zuri_packager::PackageSpecifier::fromString("foo-1.0.2@foocorp")));
}

TEST(VersionRequirement, SatisfiesNotEqual)
{
    auto req = zuri_packager::VersionRequirement::create(
        zuri_packager::PackageSpecifier::fromString("foo-1.0.1@foocorp"),
        zuri_packager::VersionComparison::NotEqual);

    ASSERT_TRUE (req->satisfiedBy(zuri_packager::PackageSpecifier::fromString("foo-1.0.2@foocorp")));
}

TEST(VersionRequirement, NotEqualNotSatisfied)
{
    auto req = zuri_packager::VersionRequirement::create(
        zuri_packager::PackageSpecifier::fromString("foo-1.0.1@foocorp"),
        zuri_packager::VersionComparison::NotEqual);

    ASSERT_FALSE (req->satisfiedBy(zuri_packager::PackageSpecifier::fromString("foo-1.0.1@foocorp")));
}
