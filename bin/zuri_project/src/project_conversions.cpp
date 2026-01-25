
#include <tempo_config/base_conversions.h>
#include <tempo_config/config_result.h>
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

    if (parameterConfig.mapContains("default")) {
        value.dfl = parameterConfig.mapAt("default");
        value.optional = true;
    } else {
        value.optional = false;
    }

    return {};
}