
#include <lyric_common/common_conversions.h>
#include <tempo_config/abstract_converter.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/config_result.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/parse_config.h>
#include <zuri_tooling/repository_store.h>
#include <zuri_tooling/tooling_conversions.h>

zuri_tooling::RepositoryStore::RepositoryStore(const tempo_config::ConfigMap &repositoriesMap)
    : m_repositoriesMap(repositoriesMap)
{
}

tempo_utils::Status
zuri_tooling::RepositoryStore::configure()
{
    tempo_config::StringParser repositoryNameParser;
    RepositoryEntryParser repositoryEntryParser;
    tempo_config::SharedPtrConstTParser sharedConstRepositoryEntryParser(&repositoryEntryParser);
    tempo_config::MapKVParser repositoryEntriesParser(&repositoryNameParser, &sharedConstRepositoryEntryParser);

    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
        m_repositoryEntries, repositoryEntriesParser, m_repositoriesMap));

    return {};
}

bool
zuri_tooling::RepositoryStore::hasRepository(const std::string &repositoryName) const
{
    return m_repositoryEntries.contains(repositoryName);
}

std::shared_ptr<const zuri_tooling::RepositoryEntry>
zuri_tooling::RepositoryStore::getRepository(const std::string &repositoryName) const
{
    auto entry = m_repositoryEntries.find(repositoryName);
    if (entry != m_repositoryEntries.cend())
        return entry->second;
    return {};
}

absl::flat_hash_map<std::string,std::shared_ptr<const zuri_tooling::RepositoryEntry>>::const_iterator
zuri_tooling::RepositoryStore::repositoriesBegin() const
{
    return m_repositoryEntries.cbegin();
}

absl::flat_hash_map<std::string,std::shared_ptr<const zuri_tooling::RepositoryEntry>>::const_iterator
zuri_tooling::RepositoryStore::repositoriesEnd() const
{
    return m_repositoryEntries.cend();
}

int
zuri_tooling::RepositoryStore::numRepositories() const
{
    return m_repositoryEntries.size();
}
