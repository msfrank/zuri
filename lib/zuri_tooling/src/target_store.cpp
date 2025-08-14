
#include <lyric_common/common_conversions.h>
#include <tempo_config/abstract_converter.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/config_result.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/parse_config.h>
#include <zuri_tooling/target_store.h>
#include <zuri_tooling/tooling_conversions.h>

zuri_tooling::TargetStore::TargetStore(const tempo_config::ConfigMap &targetsMap)
    : m_targetsMap(targetsMap)
{
}

tempo_utils::Status
zuri_tooling::TargetStore::configure()
{
    tempo_config::StringParser targetNameParser;
    TargetEntryParser targetEntryParser;
    tempo_config::SharedPtrConstTParser sharedConstTargetEntryParser(&targetEntryParser);
    tempo_config::MapKVParser targetEntriesParser(&targetNameParser, &sharedConstTargetEntryParser);

    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
        m_targetEntries, targetEntriesParser, m_targetsMap));

    return {};
}

bool
zuri_tooling::TargetStore::hasTarget(const std::string &targetName) const
{
    return m_targetEntries.contains(targetName);
}

std::shared_ptr<const zuri_tooling::TargetEntry>
zuri_tooling::TargetStore::getTarget(const std::string &targetName) const
{
    auto entry = m_targetEntries.find(targetName);
    if (entry != m_targetEntries.cend())
        return entry->second;
    return {};
}

absl::flat_hash_map<std::string,std::shared_ptr<const zuri_tooling::TargetEntry>>::const_iterator
zuri_tooling::TargetStore::targetsBegin() const
{
    return m_targetEntries.cbegin();
}

absl::flat_hash_map<std::string,std::shared_ptr<const zuri_tooling::TargetEntry>>::const_iterator
zuri_tooling::TargetStore::targetsEnd() const
{
    return m_targetEntries.cend();
}

int
zuri_tooling::TargetStore::numTargets() const
{
    return m_targetEntries.size();
}
