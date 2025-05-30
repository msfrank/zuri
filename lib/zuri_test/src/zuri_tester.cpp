#include <iostream>

#include <lyric_build/dependency_loader.h>
#include <lyric_test/mock_binder.h>
#include <lyric_test/test_result.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/chain_loader.h>
#include <lyric_test/test_inspector.h>
#include <tempo_utils/directory_maker.h>
#include <tempo_utils/tempdir_maker.h>
#include <zuri_distributor/package_cache.h>
#include <zuri_distributor/package_cache_loader.h>
#include <zuri_test/placeholder_loader.h>
#include <zuri_test/zuri_tester.h>

zuri_test::ZuriTester::ZuriTester(const TesterOptions &options)
    : m_options(options)
{
}

tempo_utils::Status
zuri_test::ZuriTester::configure()
{
    if (m_runner != nullptr)
        return lyric_test::TestStatus::forCondition(lyric_test::TestCondition::kTestInvariant,
            "tester is already configured");

    auto placeholderLoader = std::make_shared<PlaceholderLoader>();

    // construct the test runner
    auto runner = lyric_test::TestRunner::create(
        m_options.testRootDirectory,
        m_options.useInMemoryCache,
        m_options.isTemporary,
        m_options.keepBuildOnUnexpectedResult,
        m_options.taskRegistry,
        m_options.bootstrapLoader,
        placeholderLoader,
        m_options.taskSettings);

    // configure the test runner
    TU_RETURN_IF_NOT_OK (runner->configureBaseTester());

    // open the existing package cache if specified, otherwise create a new one
    std::shared_ptr<zuri_distributor::PackageCache> packageCache;
    if (m_options.existingPackageCache.empty()) {
        TU_ASSIGN_OR_RETURN (packageCache, zuri_distributor::PackageCache::create(
            runner->getTesterDirectory(), "packages"));
    } else {
        TU_ASSIGN_OR_RETURN (packageCache, zuri_distributor::PackageCache::open(
            m_options.existingPackageCache));
    }

    // install local packages
    for (const auto &packagePath : m_options.localPackages) {
        TU_RETURN_IF_STATUS (packageCache->installPackage(packagePath));
    }

    // // resolve package dependencies
    // absl::flat_hash_map<std::string,std::filesystem::path> importPackages;
    // for (const auto &entry : m_options.dependencyImports) {
    //     Option<std::filesystem::path> pathOption;
    //     TU_ASSIGN_OR_RETURN (pathOption, packageCache->resolvePackage(entry.second));
    //     if (pathOption.isEmpty())
    //         return lyric_test::TestStatus::forCondition(lyric_test::TestCondition::kTestInvariant,
    //             "missing import {}", entry.first);
    //     importPackages[entry.first] = pathOption.getValue();
    // }

    // configure requirements loader
    auto packageCacheLoader = std::make_shared<zuri_distributor::PackageCacheLoader>(packageCache);
    TU_RETURN_IF_NOT_OK (placeholderLoader->resolve(packageCacheLoader));

    m_packageCacheLoader = packageCacheLoader;
    m_packageCache = std::move(packageCache);
    m_runner = std::move(runner);
    return {};
}

const lyric_test::TestRunner *
zuri_test::ZuriTester::getRunner() const
{
    return m_runner.get();
}

tempo_utils::Result<lyric_test::RunModule>
zuri_test::ZuriTester::runModule(
    const std::string &code,
    const std::filesystem::path &modulePath,
    const std::filesystem::path &baseDir)
{
    if (!m_runner->isConfigured())
        return lyric_test::TestStatus::forCondition(lyric_test::TestCondition::kTestInvariant,
            "tester is unconfigured");

    // write the code to a module file in the src directory
    lyric_common::ModuleLocation moduleLocation;
    TU_ASSIGN_OR_RETURN (moduleLocation, m_runner->writeModuleInternal(code, modulePath, baseDir));

    lyric_build::TaskId target("compile_module", moduleLocation.toString());

    // compile the module file
    lyric_build::TargetComputationSet targetComputationSet;
    TU_ASSIGN_OR_RETURN (targetComputationSet, m_runner->computeTargetInternal(target));

    auto targetComputation = targetComputationSet.getTarget(target);
    TU_ASSERT (targetComputation.isValid());

    TU_CONSOLE_OUT << "";
    TU_CONSOLE_OUT << "======== RUN: " << moduleLocation << " ========";
    TU_CONSOLE_OUT << "";

    auto *builder = m_runner->getBuilder();
    auto cache = builder->getCache();
    auto tempRoot = builder->getTempRoot();

    // construct the loader
    auto targetState = targetComputation.getState();
    lyric_build::BuildGeneration targetGen(targetState.getGeneration());
    lyric_build::TempDirectory tempDirectory(tempRoot, targetGen);
    std::shared_ptr<lyric_build::DependencyLoader> dependencyLoader;
    TU_ASSIGN_OR_RETURN (dependencyLoader, lyric_build::DependencyLoader::create(
        targetComputation, cache, &tempDirectory));

    lyric_runtime::InterpreterStateOptions options;

    std::vector<std::shared_ptr<lyric_runtime::AbstractLoader>> loaderChain;
    loaderChain.push_back(builder->getBootstrapLoader());
    loaderChain.push_back(dependencyLoader);
    loaderChain.push_back(m_packageCacheLoader);
    options.loader = std::make_shared<lyric_runtime::ChainLoader>(loaderChain);

    // construct the interpreter state
    std::shared_ptr<lyric_runtime::InterpreterState> state;
    TU_ASSIGN_OR_RETURN (state, lyric_runtime::InterpreterState::create(options, moduleLocation));

    // run the module in the interpreter
    lyric_test::TestInspector inspector;
    lyric_runtime::BytecodeInterpreter interp(state, &inspector);
    lyric_test::MockBinder mockBinder(m_options.protocolMocks);
    lyric_runtime::InterpreterExit exit;
    TU_ASSIGN_OR_RETURN (exit, mockBinder.run(&interp));

    // return the interpreter result
    return lyric_test::RunModule(m_runner, targetComputation, targetComputationSet.getDiagnostics(), state, exit);
}

tempo_utils::Result<lyric_test::RunModule>
zuri_test::ZuriTester::runSingleModule(
    const std::string &code,
    const TesterOptions &options)
{
    ZuriTester tester(options);
    auto status = tester.configure();
    if (!status.isOk())
        return status;
    return tester.runModule(code);
}
