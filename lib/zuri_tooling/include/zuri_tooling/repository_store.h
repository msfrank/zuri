#ifndef ZURI_TOOLING_REPOSITORY_STORE_H
#define ZURI_TOOLING_REPOSITORY_STORE_H

#include <variant>

#include <tempo_config/config_types.h>
#include <tempo_utils/status.h>
#include <tempo_utils/url.h>

namespace zuri_tooling {

    enum class RepositoryEntryType {
        Invalid,
        Directory,
        Https,
    };

    struct RepositoryEntry {
        RepositoryEntryType type;
        int priority;

        struct Directory {
            std::filesystem::path directoryPath;
        };
        struct Https {
            tempo_utils::Url httpsLocation;
        };

        std::variant<Directory,Https> repository;
    };

    class RepositoryStore {
    public:
        explicit RepositoryStore(const tempo_config::ConfigMap &repositoriesMap);

        tempo_utils::Status configure();

        bool hasRepository(const std::string &repositoryName) const;
        std::shared_ptr<const RepositoryEntry> getRepository(const std::string &repositoryName) const;
        absl::flat_hash_map<std::string,std::shared_ptr<const RepositoryEntry>>::const_iterator repositoriesBegin() const;
        absl::flat_hash_map<std::string,std::shared_ptr<const RepositoryEntry>>::const_iterator repositoriesEnd() const;
        int numRepositories() const;

    private:
        tempo_config::ConfigMap m_repositoriesMap;

        absl::flat_hash_map<std::string,std::shared_ptr<const RepositoryEntry>> m_repositoryEntries;
    };
}

#endif // ZURI_TOOLING_REPOSITORY_STORE_H