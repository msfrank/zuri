
#include "lib_dir_paths.h"
#include "test_utils.h"

#include <absl/strings/str_split.h>
#include <tempo_utils/tempdir_maker.h>

std::shared_ptr<zuri_distributor::Runtime>
create_test_runtime(const std::filesystem::path &runtimeRoot)
{
    zuri_distributor::RuntimeOpenOrCreateOptions options;
    options.exclusive = true;
    options.distributionLibDir = std::filesystem::path{ZURI_BUILD_LIB_DIR};
    for (const auto &entry : absl::StrSplit(RUNTIME_LIB_DIRS, ';')) {
        std::filesystem::path libDir(entry);
        if (!libDir.is_absolute())
            continue;
        if (!std::filesystem::is_directory(libDir))
            continue;
        options.extraLibDirs.push_back(libDir);
    }
    std::shared_ptr<zuri_distributor::Runtime> runtime;
    TU_ASSIGN_OR_RAISE (runtime, zuri_distributor::Runtime::openOrCreate(runtimeRoot, options));
    return runtime;
}

static std::shared_ptr<zuri_distributor::Runtime> globalRuntime = {};
static std::mutex globalLock;

std::shared_ptr<zuri_distributor::Runtime>
get_global_test_runtime()
{
    std::lock_guard lock(globalLock);
    if (globalRuntime != nullptr)
        return globalRuntime;
    tempo_utils::TempdirMaker globalRuntimeRoot(
        std::filesystem::current_path(), "global-runtime.XXXXXXXX");
    TU_RAISE_IF_NOT_OK (globalRuntimeRoot.getStatus());
    globalRuntime = create_test_runtime(globalRuntimeRoot.getTempdir());
    return globalRuntime;
}
