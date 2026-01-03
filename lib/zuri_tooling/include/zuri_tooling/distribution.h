#ifndef ZURI_TOOLING_DISTRIBUTION_H
#define ZURI_TOOLING_DISTRIBUTION_H

#include <filesystem>

#include <tempo_utils/result.h>

namespace zuri_tooling {

    class Distribution {
    public:
        Distribution();
        Distribution(
            const std::filesystem::path &binDirectory,
            const std::filesystem::path &libDirectory,
            const std::filesystem::path &packagesDirectory,
            const std::filesystem::path &configDirectory,
            const std::filesystem::path &vendorConfigDirectory);
        Distribution(const Distribution &other);

        bool isValid() const;

        std::filesystem::path getBinDirectory() const;
        std::filesystem::path getLibDirectory() const;
        std::filesystem::path getPackagesDirectory() const;
        std::filesystem::path getConfigDirectory() const;
        std::filesystem::path getVendorConfigDirectory() const;

        static tempo_utils::Result<Distribution> load(bool ignoreMissing = false);

    private:
        struct Priv {
            std::filesystem::path binDirectory;
            std::filesystem::path libDirectory;
            std::filesystem::path packagesDirectory;
            std::filesystem::path configDirectory;
            std::filesystem::path vendorConfigDirectory;
        };
        std::shared_ptr<Priv> m_priv;
    };

}

#endif // ZURI_TOOLING_DISTRIBUTION_H