// #include <gtest/gtest.h>
// #include <gmock/gmock.h>
//
// #include <zuri_packager/package_requirement.h>
//
// #include "zuri_packager/package_dependency.h"
//
// TEST(PackageDependency, SatisfiesSingleRequirement)
// {
//     auto req = zuri_packager::ExactVersionRequirement::create(
//         1, 0, 1);
//
//     zuri_packager::PackageDependency dep("foo", "foocorp", {req});
//     ASSERT_TRUE (dep.satisfiedBy(zuri_packager::PackageSpecifier::fromString("foo-1.0.1@foocorp")));
// }
//
// TEST(PackageDependency, SatisfiesMultipleRequirements)
// {
//     auto req1 = zuri_packager::ExactVersionRequirement::create(1, 0, 0);
//     auto req2 = zuri_packager::ExactVersionRequirement::create(1, 0, 3);
//     auto req3 = zuri_packager::ExactVersionRequirement::create(2, 1, 4);
//
//     zuri_packager::PackageDependency dep("foo", "foocorp", {req1, req2, req3});
//     ASSERT_TRUE (dep.satisfiedBy(zuri_packager::PackageSpecifier::fromString("foo-1.0.3@foocorp")));
// }
