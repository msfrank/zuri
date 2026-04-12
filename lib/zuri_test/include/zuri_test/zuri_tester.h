#ifndef ZURI_TEST_ZURI_TESTER_H
#define ZURI_TEST_ZURI_TESTER_H

#include <filesystem>

#include <absl/container/flat_hash_map.h>

#include <lyric_build/task_settings.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_test/test_runner.h>
#include <zuri_distributor/runtime.h>

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
        std::vector<std::filesystem::path> localPackages = {};
        std::vector<std::string> mainArguments = {};
        absl::flat_hash_set<tempo_utils::Url> localTransportProtocols;
        absl::flat_hash_set<std::string> remoteTransportSchemes;
        absl::flat_hash_map<tempo_utils::Url,lyric_runtime::ConnectorPolicy> protocolConnectors = {};
    };

    class ZuriTester {

    public:
        ZuriTester(
            std::shared_ptr<zuri_distributor::Runtime> runtime,
            const TesterOptions &options);

        tempo_utils::Status configure();

        const lyric_test::TestRunner *getRunner() const;

        tempo_utils::Result<std::filesystem::path> writeNamedFile(
            const std::string &code,
            const std::filesystem::path &filePath,
            const std::filesystem::path &baseDir = {});
        tempo_utils::Result<std::filesystem::path> writeTempFile(
            const std::string &code,
            const std::filesystem::path &templatePath,
            const std::filesystem::path &baseDir = {});
        tempo_utils::Result<lyric_common::ModuleLocation> writeModule(
            const std::string &code,
            const std::filesystem::path &modulePath = {},
            const std::filesystem::path &baseDir = {});

        tempo_utils::Result<lyric_test::TestComputation> computeTarget(
            const lyric_build::TaskId &target,
            const lyric_build::ComputeTargetOverrides &overrides);

        tempo_utils::Result<lyric_test::RunModule> runModule(
            const std::string &code,
            const std::filesystem::path &modulePath = {},
            const std::filesystem::path &baseDir = {});

        static tempo_utils::Result<lyric_test::RunModule> runSingleModule(
            const std::string &code,
            std::shared_ptr<zuri_distributor::Runtime> runtime,
            const TesterOptions &options = {});

    private:
        std::shared_ptr<zuri_distributor::Runtime> m_runtime;
        TesterOptions m_options;

        std::shared_ptr<lyric_test::TestRunner> m_runner;
    };
}

#endif // ZURI_TEST_ZURI_TESTER_H