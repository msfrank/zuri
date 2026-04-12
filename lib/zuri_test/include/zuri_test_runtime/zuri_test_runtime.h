#ifndef ZURI_TEST_RUNTIME_ZURI_TEST_RUNTIME_H
#define ZURI_TEST_RUNTIME_ZURI_TEST_RUNTIME_H

#include <filesystem>

#include <zuri_distributor/runtime.h>

namespace zuri_test_runtime {

    std::shared_ptr<zuri_distributor::Runtime> create_test_runtime(const std::filesystem::path &runtimeRoot);

    std::shared_ptr<zuri_distributor::Runtime> get_global_test_runtime();
}

#endif // ZURI_TEST_RUNTIME_ZURI_TEST_RUNTIME_H
