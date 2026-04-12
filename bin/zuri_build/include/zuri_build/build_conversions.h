#ifndef ZURI_BUILD_BUILD_CONVERSIONS_H
#define ZURI_BUILD_BUILD_CONVERSIONS_H

#include <tempo_config/abstract_converter.h>

#include "provider_task_operations.h"

namespace zuri_build {

    class ExistingArtifactParser : public tempo_config::AbstractConverter<ExistingArtifact> {
    public:
        tempo_utils::Status parseFile(const tempo_config::ConfigMap &map, ExistingArtifact &existingArtifact) const;
        tempo_utils::Status parseExternalFile(const tempo_config::ConfigMap &map, ExistingArtifact &existingArtifact) const;
        tempo_utils::Status parseUrl(const tempo_config::ConfigMap &map, ExistingArtifact &existingArtifact) const;
        tempo_utils::Status convertValue(const tempo_config::ConfigNode &node, ExistingArtifact &existingArtifact) const override;
    };
}

#endif // ZURI_BUILD_BUILD_CONVERSIONS_H
