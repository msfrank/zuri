
#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/parse_config.h>
#include <zuri_tooling/import_store.h>
#include <zuri_tooling/tooling_conversions.h>

zuri_tooling::ImportStore::ImportStore(const tempo_config::ConfigMap &importsConfig)
    : m_importsConfig(importsConfig)
{
}

tempo_utils::Status
zuri_tooling::ImportStore::configure()
{
    tempo_config::StringParser importNameParser;
    ImportEntryParser importEntryParser;
    tempo_config::MapKVParser importEntriesParser(&importNameParser, &importEntryParser);

    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(
        m_importEntries, importEntriesParser, m_importsConfig));

    return {};
}

bool
zuri_tooling::ImportStore::hasImport(const std::string &importName) const
{
    return m_importEntries.contains(importName);
}

const zuri_tooling::ImportEntry&
zuri_tooling::ImportStore::getImport(const std::string &importName) const
{
    return m_importEntries.at(importName);
}

absl::flat_hash_map<std::string,zuri_tooling::ImportEntry>::const_iterator
zuri_tooling::ImportStore::importsBegin() const
{
    return m_importEntries.cbegin();
}

absl::flat_hash_map<std::string,zuri_tooling::ImportEntry>::const_iterator
zuri_tooling::ImportStore::importsEnd() const
{
    return m_importEntries.cend();
}

int
zuri_tooling::ImportStore::numImports() const
{
    return m_importEntries.size();
}
