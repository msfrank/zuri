#ifndef ZURI_TEST_ZURI_TESTER_H
#define ZURI_TEST_ZURI_TESTER_H

#include <filesystem>

#include <absl/container/flat_hash_map.h>

#include <lyric_build/task_settings.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_test/base_protocol_mock.h>
#include <lyric_test/test_runner.h>
#include <zuri_distributor/package_cache.h>
#include <zuri_distributor/package_cache_loader.h>
#include <zuri_packager/package_dependency.h>

namespace zuri_test {

    struct TesterOptions {
        std::filesystem::path testRootDirectory = {};
        bool useInMemoryCache = true;
        bool isTemporary = true;
        bool keepBuildOnUnexpectedResult = true;
        std::shared_ptr<lyric_build::TaskRegistry> taskRegistry = {};
        std::shared_ptr<lyric_runtime::AbstractLoader> bootstrapLoader = {};
        std::shared_ptr<lyric_runtime::AbstractLoader> fallbackLoader = {};
        lyric_build::TaskSettings taskSettings = {};
        std::filesystem::path existingPackageCache = {};
        std::vector<std::filesystem::path> localPackages;
        absl::flat_hash_map<
            tempo_utils::Url,
            std::shared_ptr<lyric_test::BaseProtocolMock>> protocolMocks = {};
    };

    class ZuriTester {

    public:
        explicit ZuriTester(const TesterOptions &options);

        tempo_utils::Status configure();

        const lyric_test::TestRunner *getRunner() const;

        tempo_utils::Result<lyric_test::RunModule> runModule(
            const std::string &code,
            const std::filesystem::path &modulePath = {},
            const std::filesystem::path &baseDir = {});

        static tempo_utils::Result<lyric_test::RunModule> runSingleModule(
            const std::string &code,
            const TesterOptions &options = {});

    private:
        TesterOptions m_options;
        std::shared_ptr<zuri_distributor::PackageCache> m_packageCache;
        std::shared_ptr<zuri_distributor::PackageCacheLoader> m_packageCacheLoader;
        std::shared_ptr<lyric_test::TestRunner> m_runner;
    };
}

#endif // ZURI_TEST_ZURI_TESTER_H