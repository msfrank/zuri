#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_test/lyric_tester.h>
#include <tempo_config/config_builder.h>
#include <tempo_config/config_utils.h>
#include <tempo_test/result_matchers.h>
#include <tempo_test/status_matchers.h>
#include <tempo_utils/tempdir_maker.h>
#include <zuri_project/template_processor.h>

class TemplateProcessorTests : public ::testing::Test {
protected:

    static std::shared_ptr<const zuri_project::ParameterEntry>
    createParam(
        const tempo_config::ConfigNode &dfl,
        bool optional)
    {
        return std::make_shared<const zuri_project::ParameterEntry>(dfl, optional);
    }
};

TEST_F(TemplateProcessorTests, RenderNoArguments)
{
    zuri_project::TemplateProcessor templateProcessor;

    auto processResult = templateProcessor.processTemplate(std::string("Hello, world!"));
    ASSERT_THAT (processResult, tempo_test::IsResult());
    auto output = processResult.getResult();

    TU_CONSOLE_ERR << output;
    ASSERT_EQ ("Hello, world!", output);
}

TEST_F(TemplateProcessorTests, RenderStringArgument)
{
    zuri_project::TemplateProcessor templateProcessor({
        {"name", createParam({}, false)},
    });

    ASSERT_THAT (templateProcessor.putArgument("name", "foobar"), tempo_test::IsOk());

    auto processResult = templateProcessor.processTemplate(std::string("Hello, {{name}}!"));
    ASSERT_THAT (processResult, tempo_test::IsResult());
    auto output = processResult.getResult();

    TU_CONSOLE_ERR << output;
    ASSERT_EQ ("Hello, foobar!", output);
}

TEST_F(TemplateProcessorTests, RenderJsonSeqArgument)
{
    zuri_project::TemplateProcessor templateProcessor({
        {"authors", createParam({}, false)},
    });

    ASSERT_THAT (templateProcessor.putArgument(
        "authors", tempo_config::startSeq()
            .append(tempo_config::valueNode("alice"))
            .append(tempo_config::valueNode("bob"))
            .append(tempo_config::valueNode("tom"))
            .buildNode()),
        tempo_test::IsOk());

    std::string templateString = R"(
{{#authors}}
author: {{.}}
{{/authors}}
)";

    auto processResult = templateProcessor.processTemplate(templateString);
    ASSERT_THAT (processResult, tempo_test::IsResult());
    auto output = processResult.getResult();

    std::string expected = R"(
author: alice
author: bob
author: tom
)";

    TU_CONSOLE_ERR << output;
    ASSERT_EQ (expected, output);
}

TEST_F(TemplateProcessorTests, RenderJsonMapArgument)
{
    zuri_project::TemplateProcessor templateProcessor({
        {"package", createParam({}, false)},
    });

    ASSERT_THAT (templateProcessor.putArgument(
        "package", tempo_config::startMap()
            .put("name", tempo_config::valueNode("foo"))
            .put("domain", tempo_config::valueNode("foocorp"))
            .put("version", tempo_config::valueNode("1.0.1"))
            .buildNode()),
        tempo_test::IsOk());

    std::string templateString = R"(package: {{package.name}}-{{package.version}}@{{package.domain}})";

    auto processResult = templateProcessor.processTemplate(templateString);
    ASSERT_THAT (processResult, tempo_test::IsResult());
    auto output = processResult.getResult();

    std::string expected = R"(package: foo-1.0.1@foocorp)";

    TU_CONSOLE_ERR << output;
    ASSERT_EQ (expected, output);
}

TEST_F(TemplateProcessorTests, RenderUndefinedContext)
{
    zuri_project::TemplateProcessor templateProcessor;

    std::string templateString = "{{missing}}";

    auto processResult = templateProcessor.processTemplate(templateString);
    ASSERT_THAT (processResult, tempo_test::IsResult());
    auto output = processResult.getResult();

    ASSERT_EQ ("", output);
}
