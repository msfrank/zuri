#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_test/lyric_tester.h>
#include <tempo_config/config_utils.h>
#include <tempo_test/result_matchers.h>
#include <tempo_test/status_matchers.h>
#include <tempo_utils/tempdir_maker.h>

#include <zuri_project/add_target.h>

class AddTargetTests : public ::testing::Test {
protected:
    std::filesystem::path outputDirectory;
    void SetUp() override {
        tempo_utils::LoggingConfiguration loggingConf;
        loggingConf.severityFilter = tempo_utils::SeverityFilter::kVeryVerbose;
        tempo_utils::init_logging(loggingConf);
        tempo_utils::TempdirMaker targetDirectoryMaker(std::filesystem::current_path(), "output.XXXXXXXX");
        TU_RAISE_IF_NOT_OK (targetDirectoryMaker.getStatus());
        outputDirectory = targetDirectoryMaker.getTempdir();
    }
    void TearDown() override {
        std::filesystem::remove_all(outputDirectory);
    }
};

TEST_F(AddTargetTests, Add)
{
    std::filesystem::path templateDirectory(TEST_TEMPLATE_DIRECTORY);
    zuri_packager::PackageSpecifier specifier("foo", "foocorp", 1, 2, 3);
    absl::flat_hash_map<std::string,std::string> userArguments;

    auto openTemplateResult = zuri_project::Template::open(templateDirectory);
    ASSERT_THAT (openTemplateResult, tempo_test::IsResult());
    auto tmpl = openTemplateResult.getResult();

    auto loadTemplateConfigResult = zuri_project::TemplateConfig::load(tmpl);
    ASSERT_THAT (loadTemplateConfigResult, tempo_test::IsResult());
    auto templateConfig = loadTemplateConfigResult.getResult();

    userArguments["foo"] = "Hello, world!";

    auto addTargetResult = zuri_project::add_target(templateConfig, "targetName",
        specifier, userArguments, outputDirectory);
    ASSERT_THAT (addTargetResult, tempo_test::IsResult());
    auto targetMap = addTargetResult.getResult();
    TU_CONSOLE_OUT << targetMap.toString();
}