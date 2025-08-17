// #include <gtest/gtest.h>
// #include <gmock/gmock.h>
//
// #include <zuri_packager/package_requirement.h>
//
// TEST(ExactVersionRequirement, ConstructsInterval)
// {
//     auto req = zuri_packager::ExactVersionRequirement::create(
//         1, 2, 3);
//
//     auto interval = req->getInterval();
//     ASSERT_EQ (interval.closedLowerBound, zuri_packager::PackageVersion(1, 2, 3));
//     ASSERT_EQ (interval.openUpperBound, zuri_packager::PackageVersion(1, 2, 4));
// }
//
// TEST(CaretRangeRequirement, ConstructsIntervalForFullVersion)
// {
//     auto req = zuri_packager::CaretRangeRequirement::create(
//         1, 2, 3);
//
//     auto interval = req->getInterval();
//     ASSERT_EQ (interval.closedLowerBound, zuri_packager::PackageVersion(1, 2, 3));
//     ASSERT_EQ (interval.openUpperBound, zuri_packager::PackageVersion(2, 0, 0));
// }
//
// TEST(CaretRangeRequirement, ConstructsIntervalForApiVersion)
// {
//     auto req = zuri_packager::CaretRangeRequirement::create(
//         1, 2);
//
//     auto interval = req->getInterval();
//     ASSERT_EQ (interval.closedLowerBound, zuri_packager::PackageVersion(1, 2, 0));
//     ASSERT_EQ (interval.openUpperBound, zuri_packager::PackageVersion(2, 0, 0));
// }
//
