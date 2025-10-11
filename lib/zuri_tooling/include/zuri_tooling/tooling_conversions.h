#ifndef ZURI_TOOLING_TOOLING_CONVERSIONS_H
#define ZURI_TOOLING_TOOLING_CONVERSIONS_H

#include <lyric_build/lyric_builder.h>
#include <tempo_config/abstract_converter.h>
#include <tempo_config/enum_conversions.h>

#include "import_store.h"
#include "package_store.h"
#include "target_store.h"

namespace zuri_tooling {

    class ImportEntryParser : public tempo_config::AbstractConverter<ImportEntry> {
    public:
        tempo_utils::Status parseString(const tempo_config::ConfigValue &value, ImportEntry &importEntry) const;
        tempo_utils::Status parseVersion(const tempo_config::ConfigMap &map, ImportEntry &importEntry) const;
        tempo_utils::Status parsePath(const tempo_config::ConfigMap &map, ImportEntry &importEntry) const;

        tempo_utils::Status convertValue(const tempo_config::ConfigNode &node, ImportEntry &importEntry) const override;
    };

    class TargetEntryParser : public tempo_config::AbstractConverter<TargetEntry> {
    public:
        tempo_utils::Status parseProgram(const tempo_config::ConfigMap &map, TargetEntry &targetEntry) const;
        tempo_utils::Status parseLibrary(const tempo_config::ConfigMap &map, TargetEntry &targetEntry) const;
        tempo_utils::Status parsePackage(const tempo_config::ConfigMap &map, TargetEntry &targetEntry) const;

        tempo_utils::Status convertValue(const tempo_config::ConfigNode &node, TargetEntry &targetEntry) const override;
    };

    class PackageCacheEntryParser : public tempo_config::AbstractConverter<PackageCacheEntry> {
    public:
        tempo_utils::Status convertValue(
            const tempo_config::ConfigNode &node,
            PackageCacheEntry &packageCacheEntry) const override;
    };

    /**
     *
     */
    class CacheModeParser : public tempo_config::EnumTParser<lyric_build::CacheMode> {
    public:
        explicit CacheModeParser(lyric_build::CacheMode defaultMode)
            : EnumTParser({
            {"Default", lyric_build::CacheMode::Default},
            {"Persistent", lyric_build::CacheMode::Persistent},
            {"InMemory", lyric_build::CacheMode::InMemory}}, defaultMode)
        {}
        CacheModeParser() : CacheModeParser(lyric_build::CacheMode::Default) {}
    };
}

#endif // ZURI_TOOLING_TOOLING_CONVERSIONS_H