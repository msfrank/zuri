#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_bootstrap/bootstrap_helpers.h>
#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_config/workspace_config.h>
#include <tempo_test/tempo_test.h>
#include <tempo_utils/file_utilities.h>
#include <tempo_utils/file_reader.h>
#include <tempo_utils/file_writer.h>
#include <zuri_test/zuri_tester.h>

#include "test_utils.h"

class FsFile : public ::testing::Test {
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

TEST_F(FsFile, EvaluateCreateFile)
{
    auto filename = tempo_utils::generate_name("create_file.XXXXXXXX");
    auto path = std::filesystem::absolute(filename);

    auto result = tester->runModule(absl::StrFormat(R"(
        import from "dev.zuri.pkg://fs-0.0.1@zuri.dev/file" ...

        val file: File = File{"%s"}
        file.Create(ReadWrite)
    )", path.c_str()));

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
    DataCellRef(lyric_common::SymbolUrl(
        lyric_common::ModuleLocation::fromString("dev.zuri.pkg://fs-0.0.1@zuri.dev/file"),
        lyric_common::SymbolPath({"File"}))))));

    ASSERT_TRUE (std::filesystem::exists(path));
}

TEST_F(FsFile, EvaluateOpenFile)
{
    auto filename = tempo_utils::generate_name("open_file.XXXXXXXX");
    auto path = std::filesystem::absolute(filename);

    std::string content("hello, world!");
    tempo_utils::FileWriter writer(path, content, tempo_utils::FileWriterMode::CREATE_OR_OVERWRITE);
    TU_RAISE_IF_NOT_OK (writer.getStatus());

    auto result = tester->runModule(absl::StrFormat(R"(
        import from "dev.zuri.pkg://fs-0.0.1@zuri.dev/file" ...
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/system" ...

        val file: File = expect File{"%s"}.Open(ReadOnly)
        Await(file.Read(512))
    )", path.c_str()));

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        DataCellBytes(content))));
}

TEST_F(FsFile, EvaluateCreateFileAndWrite)
{
    auto filename = tempo_utils::generate_name("create_and_write.XXXXXXXX");
    auto path = std::filesystem::absolute(filename);
    std::string content("hello, world!");

    auto result = tester->runModule(absl::StrFormat(R"(
        import from "dev.zuri.pkg://fs-0.0.1@zuri.dev/file" ...
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/system" ...

        val file: File = expect File{"%s"}.Create(ReadWrite)
        Await(file.Write("%s".ToBytes()))
    )", path.c_str(), content));

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        DataCellInt(13))));

    tempo_utils::FileReader reader(path);
    ASSERT_THAT (reader.getStatus(), tempo_test::IsOk());
    auto bytes = reader.getBytes();
    ASSERT_EQ (content, std::string_view((const char *) bytes->getData(), bytes->getSize()));
}
