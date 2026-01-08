#ifndef ZURI_TOOLING_HOME_H
#define ZURI_TOOLING_HOME_H

#include <filesystem>

#include <tempo_utils/result.h>

namespace zuri_tooling {

    class Home {
    public:
        Home();
        Home(
            const std::filesystem::path &homeDirectory,
            const std::filesystem::path &packagesDirectory,
            const std::filesystem::path &environmentsDirectory,
            const std::filesystem::path &configDirectory,
            const std::filesystem::path &vendorConfigDirectory);
        Home(const Home &other);

        bool isValid() const;

        std::filesystem::path getHomeDirectory() const;
        std::filesystem::path getPackagesDirectory() const;
        std::filesystem::path getEnvironmentsDirectory() const;
        std::filesystem::path getConfigDirectory() const;
        std::filesystem::path getVendorConfigDirectory() const;

        static tempo_utils::Result<Home> openOrCreate(const std::filesystem::path &homeDirectory = {});
        static tempo_utils::Result<Home> open(const std::filesystem::path &homeDirectory, bool ignoreMissing = false);
        static tempo_utils::Result<Home> open(bool ignoreMissing = false);

    private:
        struct Priv {
            std::filesystem::path homeDirectory;
            std::filesystem::path packagesDirectory;
            std::filesystem::path environmentsDirectory;
            std::filesystem::path configDirectory;
            std::filesystem::path vendorConfigDirectory;
        };
        std::shared_ptr<Priv> m_priv;
    };

    const char *homeDirectoryName();

}

#endif // ZURI_TOOLING_HOME_H