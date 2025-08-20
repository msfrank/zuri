#ifndef ZURI_TOOLING_TARGET_STORE_H
#define ZURI_TOOLING_TARGET_STORE_H

#include <variant>

#include <lyric_common/module_location.h>
#include <tempo_config/config_types.h>
#include <tempo_utils/status.h>
#include <zuri_packager/package_specifier.h>

namespace zuri_tooling {

    enum class TargetEntryType {
        Invalid,
        Library,
        Program,
        Package,
    };

    struct TargetEntry {
        TargetEntryType type;
        std::vector<std::string> depends;

        struct Program {
            zuri_packager::PackageSpecifier specifier;
            std::vector<lyric_common::ModuleLocation> modules;
            lyric_common::ModuleLocation main;
        };
        struct Library {
            zuri_packager::PackageSpecifier specifier;
            std::vector<lyric_common::ModuleLocation> modules;
        };
        struct Package {
            tempo_utils::Url url;
        };

        std::variant<Library,Program,Package> target;
    };

    class TargetStore {
    public:
        explicit TargetStore(const tempo_config::ConfigMap &targetsMap);

        tempo_utils::Status configure();

        bool hasTarget(const std::string &targetName) const;
        std::shared_ptr<const TargetEntry> getTarget(const std::string &targetName) const;
        absl::flat_hash_map<std::string,std::shared_ptr<const TargetEntry>>::const_iterator targetsBegin() const;
        absl::flat_hash_map<std::string,std::shared_ptr<const TargetEntry>>::const_iterator targetsEnd() const;
        int numTargets() const;

    private:
        tempo_config::ConfigMap m_targetsMap;

        absl::flat_hash_map<std::string,std::shared_ptr<const TargetEntry>> m_targetEntries;
    };
}

#endif // ZURI_TOOLING_TARGET_STORE_H
