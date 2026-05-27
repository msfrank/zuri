#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_build/build_attrs.h>
#include <lyric_common/common_types.h>
#include <tempo_config/config_utils.h>
#include <tempo_test/result_matchers.h>
#include <tempo_test/status_matchers.h>
#include <zuri_build/generate_protocol_task.h>

#include "base_task_fixture.h"

class GenerateProtocolTask : public BaseTaskFixture {};

TEST_F(GenerateProtocolTask, TaskSucceeds)
{
    tempo_config::ConfigNode rootNode;
    TU_ASSIGN_OR_RAISE (rootNode, tempo_config::read_config_string(R"({
            "global": {},
            "domain": {},
            "tasks": {
                "generate_protocol:/protocol": {
                    "protocolName": "TestProtocol",
                    "protocolType": "Connect",
                    "communicationDirection": "Send",
                    "sends": "#Bytes",
                    "receives": null,
                }
            }
        })"));
    taskSettings = lyric_build::TaskSettings(rootNode.toMap());

    lyric_build::TaskKey key(std::string("generate_protocol"), std::string("/protocol"));
    auto *task = zuri_build::new_generate_protocol_task(generation, key, buildState, span);
    lyric_build::TaskLocker locker(task);

    ASSERT_THAT (task->configureTask(taskSettings), tempo_test::IsOk());

    lyric_build::TaskHash taskHash;
    ASSERT_THAT (task->deduplicateTask(taskHash), tempo_test::IsOk());
    ASSERT_TRUE (taskHash.isValid());
    ASSERT_THAT (task->setHash(taskHash), tempo_test::IsResult());

    auto *tmp = tempDirectory();
    ASSERT_THAT (task->runTask(tmp), tempo_test::IsOk());

    auto artifactCache = buildState->getArtifactCache();
    lyric_build::ArtifactId artifactId(generation, taskHash, tempo_utils::Url::fromString("/protocol.lyo"));

    auto loadMetadataResult = artifactCache->loadMetadata(artifactId);
    ASSERT_THAT (loadMetadataResult, tempo_test::IsResult());
    auto metadata = loadMetadataResult.getResult();

    std::string contentType;
    metadata.parseAttr(lyric_build::kLyricBuildContentType, contentType);
    ASSERT_EQ (lyric_common::kObjectContentType, contentType);

    auto loadContentResult = artifactCache->loadContent(artifactId);
    ASSERT_THAT (loadContentResult, tempo_test::IsResult());
    auto content = loadContentResult.getResult();

    lyric_object::LyricObject object(content);
    ASSERT_TRUE (object.isValid());
}

// TEST_F(GenerateProtocolTask, ConfigureTaskFailsWhenSourceFileIsMissing)
// {
//     lyric_build::TaskKey key(std::string("parse_archetype"), std::string("/mod"));
//     auto *task = lyric_build::internal::new_parse_archetype_task(generation, key, buildState, span);
//     auto status = task->configureTask(taskSettings);
//     ASSERT_THAT (status, tempo_test::ContainsStatus(lyric_build::BuildCondition::kMissingInput));
// }
