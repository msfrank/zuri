#ifndef ZURI_TOOLING_TOOLING_CONVERSIONS_H
#define ZURI_TOOLING_TOOLING_CONVERSIONS_H

#include <tempo_config/abstract_converter.h>

#include "import_store.h"
#include "target_store.h"

namespace zuri_tooling {

    class ImportEntryParser : public tempo_config::AbstractConverter<ImportEntry> {
    public:
        static tempo_utils::Status parseTarget(const tempo_config::ConfigMap &map, ImportEntry &importEntry);
        static tempo_utils::Status parseRequirement(const tempo_config::ConfigMap &map, ImportEntry &importEntry);
        static tempo_utils::Status parsePackage(const tempo_config::ConfigMap &map, ImportEntry &importEntry);

        tempo_utils::Status convertValue(const tempo_config::ConfigNode &node, ImportEntry &importEntry) const override;
    };

    class TargetEntryParser : public tempo_config::AbstractConverter<TargetEntry> {
    public:
        tempo_utils::Status parseProgram(const tempo_config::ConfigMap &map, TargetEntry &targetEntry) const;
        tempo_utils::Status parseLibrary(const tempo_config::ConfigMap &map, TargetEntry &targetEntry) const;
        tempo_utils::Status parseArchive(const tempo_config::ConfigMap &map, TargetEntry &targetEntry) const;

        tempo_utils::Status convertValue(const tempo_config::ConfigNode &node, TargetEntry &targetEntry) const override;
    };
}
#endif // ZURI_TOOLING_TOOLING_CONVERSIONS_H