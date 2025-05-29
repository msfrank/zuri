#ifndef ZURI_DISTRIBUTOR_DISTRIBUTION_BUILDER_H
#define ZURI_DISTRIBUTOR_DISTRIBUTION_BUILDER_H
#include <__filesystem/filesystem_error.h>

#include "distributor_result.h"

namespace zuri_distributor {

    struct DistributionBuilderOptions {
    };

    class DistributionBuilder {
    public:
        explicit DistributionBuilder(
            const std::filesystem::path &distributionRoot,
            const DistributionBuilderOptions &options = {});

        tempo_utils::Status configure();

    private:
        std::filesystem::path m_distributionRoot;
        DistributionBuilderOptions m_options;
    };
}

#endif // ZURI_DISTRIBUTOR_DISTRIBUTION_BUILDER_H
