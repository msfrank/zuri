#include <gtest/gtest.h>

#include <zuri_packager/package_specifier.h>

TEST(PackageSpecifier, TestParsePackageSpecifier)
{
    auto url = tempo_utils::Url::fromString("dev.zuri.pkg://test-1.2.3@zuri.dev");
    ASSERT_TRUE (url.isValid());
    auto specifier = zuri_packager::PackageSpecifier::fromUrl(url);
    ASSERT_TRUE (specifier.isValid());
    ASSERT_EQ ("test", specifier.getPackageName());
    ASSERT_EQ ("zuri.dev", specifier.getPackageDomain());
    ASSERT_EQ (1, specifier.getMajorVersion());
    ASSERT_EQ (2, specifier.getMinorVersion());
    ASSERT_EQ (3, specifier.getPatchVersion());
}
