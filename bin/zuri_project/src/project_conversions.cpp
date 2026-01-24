
#include <tempo_config/base_conversions.h>
#include <tempo_config/config_result.h>
#include <tempo_config/enum_conversions.h>
#include <tempo_config/parse_config.h>
#include <zuri_project/project_conversions.h>

tempo_utils::Status
zuri_project::ParameterEntryParser::convertValue(
    const tempo_config::ConfigNode &node,
    ParameterEntry &value) const
{
    if (node.getNodeType() != tempo_config::ConfigNodeType::kMap)
        return tempo_config::ConfigStatus::forCondition(
            tempo_config::ConfigCondition::kWrongType, "parameter entry config must be a map");
    auto parameterConfig = node.toMap();

    tempo_config::EnumTParser<tempo_config::ConfigNodeType> typeParser({
        {"String", tempo_config::ConfigNodeType::kValue},
        {"Int", tempo_config::ConfigNodeType::kValue},
        {"Bool", tempo_config::ConfigNodeType::kValue},
        {"Seq", tempo_config::ConfigNodeType::kSeq},
        {"Map", tempo_config::ConfigNodeType::kMap},
    });

    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(value.type, typeParser,
        parameterConfig, "type"));
    if (parameterConfig.mapContains("default")) {
        value.dfl = parameterConfig.mapAt("default");
        value.optional = true;
    } else {
        value.optional = false;
    }

    return {};
}