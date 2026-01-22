#ifndef ZURI_PROJECT_PROJECT_CONVERSIONS_H
#define ZURI_PROJECT_PROJECT_CONVERSIONS_H

#include <tempo_config/abstract_converter.h>

#include "template_config.h"

namespace zuri_project {

    class ParameterEntryParser : public tempo_config::AbstractConverter<ParameterEntry> {
    public:
        tempo_utils::Status convertValue(const tempo_config::ConfigNode &node, ParameterEntry &value) const override;
    };

    template<class K, class V>
    class PairKVParser : public tempo_config::AbstractConverter<std::pair<K, V>> {

    public:
        PairKVParser(
            const tempo_config::AbstractConverter<K> *keyParser,
            const tempo_config::AbstractConverter<V> *valueParser,
            char separator = ':')
            : m_keyParser(keyParser), m_valueParser(valueParser), m_separator(separator)
        {
            TU_ASSERT (m_keyParser != nullptr);
            TU_ASSERT (m_valueParser != nullptr);
            TU_ASSERT (std::isprint(m_separator));
        }
        PairKVParser(
            const tempo_config::AbstractConverter<K> *keyParser,
            const tempo_config::AbstractConverter<V> *valueParser,
            const absl::flat_hash_map<K, V> &mapDefault,
            char separator = ':')
            : m_keyParser(keyParser),
              m_valueParser(valueParser),
              m_default(mapDefault),
              m_separator(separator)
        {
            TU_ASSERT (m_keyParser != nullptr);
            TU_ASSERT (m_valueParser != nullptr);
            TU_ASSERT (std::isprint(m_separator));
        }

        tempo_utils::Status convertValue(
            const tempo_config::ConfigNode &node,
            std::pair<K, V> &pair) const override
        {
            if (node.isNil()) {
                if (m_default.isEmpty())
                    return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kMissingValue,
                        "missing required value");
                pair = m_default.getValue();
                return {};
            }
            if (node.getNodeType() != tempo_config::ConfigNodeType::kValue)
                return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kWrongType,
                    "expected Value node but found {}",
                    tempo_config::config_node_type_to_string(node.getNodeType()));
            auto nodeValue = node.toValue().getValue();

            auto index = nodeValue.find(m_separator);
            if (index == std::string::npos)
                return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kParseError,
                    "missing key-value separator '{}' in '{}'", m_separator, nodeValue);
            auto keyString = nodeValue.substr(0, index);
            auto valueString = nodeValue.substr(index + 1);

            K key;
            V value;
            tempo_utils::Status status;
            TU_RETURN_IF_NOT_OK (m_keyParser->convertValue(
                tempo_config::ConfigValue(std::move(keyString)), key));
            TU_RETURN_IF_NOT_OK (m_valueParser->convertValue(
                tempo_config::ConfigValue(std::move(valueString)), value));
            pair = std::pair(key, value);
            return {};
        }

    private:
        const tempo_config::AbstractConverter<K> *m_keyParser;
        const tempo_config::AbstractConverter<V> *m_valueParser;
        Option<std::pair<K, V>> m_default;
        char m_separator;
    };
}

#endif // ZURI_PROJECT_PROJECT_CONVERSIONS_H