#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_test/lyric_tester.h>
#include <tempo_config/config_builder.h>
#include <tempo_config/config_utils.h>
#include <tempo_test/result_matchers.h>
#include <tempo_test/status_matchers.h>
#include <tempo_utils/file_writer.h>
#include <tempo_utils/tempdir_maker.h>
#include <zuri_project/update_requirements.h>
#include <zuri_tooling/project.h>
#include <zuri_tooling/tooling_conversions.h>

class UpdateRequirementsTests : public ::testing::Test {
protected:
    std::filesystem::path outputDirectory;
    std::filesystem::path projectConfigFile;
    void SetUp() override {
        tempo_utils::LoggingConfiguration loggingConf;
        loggingConf.severityFilter = tempo_utils::SeverityFilter::kVeryVerbose;
        tempo_utils::init_logging(loggingConf);
        tempo_utils::TempdirMaker targetDirectoryMaker(std::filesystem::current_path(), "output.XXXXXXXX");
        TU_RAISE_IF_NOT_OK (targetDirectoryMaker.getStatus());
        outputDirectory = targetDirectoryMaker.getTempdir();
        projectConfigFile = outputDirectory / zuri_tooling::kProjectConfigName;
        TU_RAISE_IF_NOT_OK (tempo_config::write_config_file(tempo_config::ConfigMap{}, projectConfigFile));
    }
    void TearDown() override {
        std::filesystem::remove_all(outputDirectory);
    }
};

TEST_F(UpdateRequirementsTests, AddRequirementToEmptyImportsMap)
{
    std::filesystem::path templateDirectory(TEST_TEMPLATE_DIRECTORY);
    zuri_packager::PackageSpecifier specifier("foo", "foocorp", 1, 2, 3);

    auto openTemplateResult = zuri_project::Template::open(templateDirectory);
    ASSERT_THAT (openTemplateResult, tempo_test::IsResult());
    auto tmpl = openTemplateResult.getResult();

    auto loadTemplateConfigResult = zuri_project::TemplateConfig::load(tmpl);
    ASSERT_THAT (loadTemplateConfigResult, tempo_test::IsResult());
    auto templateConfig = loadTemplateConfigResult.getResult();

    zuri_project::UpdateRequirementsOperation updateRequirementsOp;
    updateRequirementsOp.addRequirements = {
        specifier,
    };

    auto importsMap = tempo_config::startMap()
        .buildMap();
    auto importStore = std::make_shared<zuri_tooling::ImportStore>(importsMap);

    TU_RAISE_IF_NOT_OK (tempo_config::write_config_file(
        tempo_config::startMap()
            .put("imports", importsMap)
        .buildNode(),
        projectConfigFile));

    tempo_config::ConfigFileEditor projectConfigEditor(projectConfigFile);
    ASSERT_THAT (zuri_project::update_requirements(updateRequirementsOp, importStore, projectConfigEditor), tempo_test::IsOk());
    ASSERT_THAT (projectConfigEditor.writeFile(), tempo_test::IsOk());

    tempo_config::ConfigMap projectMap;
    TU_ASSIGN_OR_RAISE (projectMap, tempo_config::read_config_map_file(projectConfigFile));
    TU_CONSOLE_OUT << projectMap.toString();

    auto projectImportsMap = projectMap.mapAt("imports").toMap();
    ASSERT_TRUE (projectImportsMap.mapContains(specifier.getPackageId().toString()));
    ASSERT_EQ (tempo_config::valueNode(specifier.getVersionString()), projectImportsMap.mapAt(specifier.getPackageId().toString()));
}
