#include <gtest/gtest.h>

#include <zuri_packager/package_reader.h>
#include <zuri_packager/package_writer.h>
#include <lyric_test/lyric_tester.h>
#include <tempo_utils/tempdir_maker.h>

#include "package_attrs.h"

TEST(PackageLoader, TestLoadAssembly)
{
    lyric_test::TesterOptions options;
    lyric_test::LyricTester tester(options);
    auto compileModuleResult = tester.compileSingleModule("42");
    ASSERT_TRUE (compileModuleResult.isResult());
    auto object = compileModuleResult.getResult();

    tempo_utils::TempdirMaker tempdirMaker(std::filesystem::current_path(), "test_PackageWriter.XXXXXXXX");
    ASSERT_TRUE (tempdirMaker.isValid());
    zuri_packager::PackageWriter writer;
    ASSERT_TRUE (writer.configure().isOk());
    writer.putFile(zuri_packager::EntryPath::fromString("/test.lyo"), object.bytesView());
    writer.putPackageAttr(zuri_packager::kLyricPackagingMainLocation, lyric_common::AssemblyLocation::fromString("/test"));
    auto packagePath = tempdirMaker.getTempdir() / "dev.zuri_package-0.0.0.zpk";
    auto status = writer.writePackage(packagePath);
    ASSERT_TRUE (status.isOk());
    ASSERT_TRUE (exists(packagePath));

    auto location = lyric_common::AssemblyLocation::fromString("dev.zuri.pkg://package-0.0.0@zuri.dev/test");
    ASSERT_TRUE (location.isValid());

    zuri_packager::PackageLoader loader({tempdirMaker.getTempdir()});
    ASSERT_TRUE (loader.hasModule(location));
    auto loadedAssembly = loader.loadModule(location);
    ASSERT_TRUE (loadedAssembly.isValid());

    ASSERT_TRUE (remove(packagePath));
}