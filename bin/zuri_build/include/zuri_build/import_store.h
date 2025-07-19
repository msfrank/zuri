#ifndef ZURI_BUILD_IMPORT_STORE_H
#define ZURI_BUILD_IMPORT_STORE_H

#include <lyric_build/build_types.h>

#include "zuri_packager/package_requirement.h"

enum class ImportEntryType {
    Invalid,
    Target,
    Requirement,
    Package,
};

struct ImportEntry {
    ImportEntryType type;
    std::string targetName;
    zuri_packager::PackageSpecifier requirementSpecifier;
    tempo_utils::Url packageUrl;
};

class ImportStore {
public:
    explicit ImportStore(const tempo_config::ConfigMap &importsConfig);

    tempo_utils::Status configure();

    bool hasImport(const std::string &importName) const;
    const ImportEntry& getImport(const std::string &importName) const;
    absl::flat_hash_map<std::string,ImportEntry>::const_iterator importsBegin() const;
    absl::flat_hash_map<std::string,ImportEntry>::const_iterator importsEnd() const;
    int numImports() const;

private:
    tempo_config::ConfigMap m_importsConfig;

    absl::flat_hash_map<std::string,ImportEntry> m_importEntries;
};

#endif // ZURI_BUILD_IMPORT_STORE_H
