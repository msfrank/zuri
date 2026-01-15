#ifndef DEV_ZURI_TEST_UTILS_H
#define DEV_ZURI_TEST_UTILS_H

#include <zuri_distributor/runtime.h>

std::shared_ptr<zuri_distributor::Runtime> create_test_runtime(
    const std::filesystem::path &runtimeRoot);

std::shared_ptr<zuri_distributor::Runtime> get_global_test_runtime();

#endif // DEV_ZURI_TEST_UTILS_H