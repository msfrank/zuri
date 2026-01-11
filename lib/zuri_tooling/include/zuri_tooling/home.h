#ifndef ZURI_TOOLING_HOME_H
#define ZURI_TOOLING_HOME_H

#include <filesystem>

#include <tempo_utils/result.h>

namespace zuri_tooling {

    /**
     *
     */
    constexpr const char * const kEnvHomeName = "ZURI_HOME";

    class Home {
    public:
        Home();
        Home(const Home &other);

        bool isValid() const;

        std::filesystem::path getHomeDirectory() const;
        std::filesystem::path getConfigDirectory() const;
        std::filesystem::path getCacheDirectory() const;

        static tempo_utils::Result<Home> openOrCreate(const std::filesystem::path &homeDirectory = {});
        static tempo_utils::Result<Home> open(const std::filesystem::path &homeDirectory, bool ignoreMissing = false);
        static tempo_utils::Result<Home> open(bool ignoreMissing = false);

    private:
        struct Priv {
            std::filesystem::path homeDirectory;
            std::filesystem::path configDirectory;
            std::filesystem::path cacheDirectory;
        };
        std::shared_ptr<Priv> m_priv;

        Home(
            const std::filesystem::path &homeDirectory,
            const std::filesystem::path &configDirectory,
            const std::filesystem::path &cacheDirectory);
    };

    const char *homeDirectoryName();
}

#endif // ZURI_TOOLING_HOME_H