#ifndef ZURI_TEST_PLACEHOLDER_LOADER_H
#define ZURI_TEST_PLACEHOLDER_LOADER_H
#include <lyric_runtime/abstract_loader.h>

namespace zuri_test {

    class PlaceholderLoader : public lyric_runtime::AbstractLoader {
    public:
        PlaceholderLoader() = default;

        tempo_utils::Status resolve(std::shared_ptr<lyric_runtime::AbstractLoader> loader);

        tempo_utils::Result<bool> hasModule(const lyric_common::ModuleLocation &location) const override;

        tempo_utils::Result<Option<lyric_object::LyricObject>> loadModule(
            const lyric_common::ModuleLocation &location) override;

        tempo_utils::Result<Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>> loadPlugin(
            const lyric_common::ModuleLocation &location,
            const lyric_object::PluginSpecifier &specifier) override;

    private:
        std::shared_ptr<AbstractLoader> m_loader;
    };
}

#endif // ZURI_TEST_PLACEHOLDER_LOADER_H
