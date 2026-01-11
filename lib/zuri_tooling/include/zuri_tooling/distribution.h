#ifndef ZURI_TOOLING_DISTRIBUTION_H
#define ZURI_TOOLING_DISTRIBUTION_H

#include <filesystem>

#include <tempo_utils/result.h>

namespace zuri_tooling {

    class Distribution {
    public:
        Distribution();
        Distribution(const Distribution &other);

        bool isValid() const;

        std::filesystem::path getBinDirectory() const;
        std::filesystem::path getLibDirectory() const;
        std::filesystem::path getConfigDirectory() const;

        static tempo_utils::Result<Distribution> open();
        static tempo_utils::Result<Distribution> find(const std::filesystem::path &programLocation);

    private:
        struct Priv {
            std::filesystem::path binDirectory;
            std::filesystem::path libDirectory;
            std::filesystem::path configDirectory;
        };
        std::shared_ptr<Priv> m_priv;

        Distribution(
            const std::filesystem::path &binDirectory,
            const std::filesystem::path &libDirectory,
            const std::filesystem::path &configDirectory);
    };
}

#endif // ZURI_TOOLING_DISTRIBUTION_H