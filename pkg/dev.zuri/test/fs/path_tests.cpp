#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>
#include <tempo_utils/file_reader.h>
#include <zuri_test/zuri_tester.h>

#include "test_utils.h"

class FsPath : public ::testing::Test {
protected:
    std::unique_ptr<zuri_test::ZuriTester> tester;

    void SetUp() override {
        auto runtime = get_global_test_runtime();
        zuri_test::TesterOptions options;
        options.localPackages.emplace_back(ZURI_STD_PACKAGE_PATH);
        options.localPackages.emplace_back(ZURI_FS_PACKAGE_PATH);
        tester = std::make_unique<zuri_test::ZuriTester>(runtime, options);
        TU_RAISE_IF_NOT_OK (tester->configure());
    }
};

TEST_F(FsPath, EvaluateCurrentPath)
{
    auto path = std::filesystem::current_path();

    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://fs-0.0.1@zuri.dev/path" ...

        val path = Path.Current{}
        path.ToString()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
    DataCellString(path.string()))));
}

TEST_F(FsPath, EvaluateIsAbsolute)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://fs-0.0.1@zuri.dev/path" ...

        val path = Path{"/parent/filename.txt"}
        path.IsAbsolute() and not path.IsRelative()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
    DataCellBool(true))));
}

TEST_F(FsPath, EvaluateIsRelative)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://fs-0.0.1@zuri.dev/path" ...

        val path = Path{"filename.txt"}
        path.IsRelative() and not path.IsAbsolute()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
    DataCellBool(true))));
}

TEST_F(FsPath, EvaluateFileName)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://fs-0.0.1@zuri.dev/path" ...

        val path = Path{"/parent/filename.txt"}
        path.FileName().ToString()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
    DataCellString("filename.txt"))));
}

TEST_F(FsPath, EvaluateFileStem)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://fs-0.0.1@zuri.dev/path" ...

        val path = Path{"/parent/filename.txt"}
        path.FileStem().ToString()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
    DataCellString("filename"))));
}

TEST_F(FsPath, EvaluateFileExtension)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://fs-0.0.1@zuri.dev/path" ...

        val path = Path{"/parent/filename.txt"}
        path.FileExtension()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
    DataCellString(".txt"))));
}

TEST_F(FsPath, EvaluateParent)
{
    std::filesystem::path path("/parent/filename.txt");

    auto result = tester->runModule(absl::StrFormat(R"(
        import from "dev.zuri.pkg://fs-0.0.1@zuri.dev/path" ...

        val path = Path{"%s"}
        path.Parent().ToString()
    )", path.string()));

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
    DataCellString(path.parent_path().string()))));
}

TEST_F(FsPath, EvaluateResolve)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://fs-0.0.1@zuri.dev/path" ...

        val path = Path{"/parent/"}
        val resolved = path.Resolve("foo/bar", "../newname.txt")
        resolved.ToString()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
    DataCellString("/parent/foo/newname.txt"))));
}
