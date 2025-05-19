#ifndef ZURI_BUILD_TARGET_STORE_H
#define ZURI_BUILD_TARGET_STORE_H

#include <lyric_common/common_types.h>
#include <lyric_common/module_location.h>
#include <tempo_config/config_types.h>
#include <tempo_utils/attr.h>

enum class TargetEntryType {
    Invalid,
    Library,
    Program,
    Archive,
};

struct TargetEntry {
    TargetEntryType type;
    std::string version;
    std::vector<std::string> depends;
    std::vector<std::string> imports;
    std::vector<lyric_common::ModuleLocation> modules;
    lyric_common::ModuleLocation main;
};

class TargetStore {
public:
    explicit TargetStore(const tempo_config::ConfigMap &targetsConfig);

    tempo_utils::Status configure();

    bool hasTarget(const std::string &targetName) const;
    const TargetEntry& getTarget(const std::string &targetName) const;
    absl::flat_hash_map<std::string,TargetEntry>::const_iterator targetsBegin() const;
    absl::flat_hash_map<std::string,TargetEntry>::const_iterator targetsEnd() const;
    int numTargets() const;

private:
    tempo_config::ConfigMap m_targetsConfig;

    absl::flat_hash_map<std::string,TargetEntry> m_targetEntries;
};

#endif // ZURI_BUILD_TARGET_STORE_H
