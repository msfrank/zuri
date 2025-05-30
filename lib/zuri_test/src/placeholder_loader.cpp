
#include <lyric_runtime/interpreter_result.h>
#include <zuri_test/placeholder_loader.h>

tempo_utils::Status
zuri_test::PlaceholderLoader::resolve(std::shared_ptr<lyric_runtime::AbstractLoader> loader)
{
    if (m_loader != nullptr)
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant,
            "placeholder loader is already resolved");
    m_loader = std::move(loader);
    return {};
}

tempo_utils::Result<bool>
zuri_test::PlaceholderLoader::hasModule(const lyric_common::ModuleLocation &location) const
{
    if (m_loader == nullptr)
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant,
            "placeholder loader is not resolved");
    return m_loader->hasModule(location);
}

tempo_utils::Result<Option<lyric_object::LyricObject>>
zuri_test::PlaceholderLoader::loadModule(const lyric_common::ModuleLocation &location)
{
    if (m_loader == nullptr)
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant,
            "placeholder loader is not resolved");
    return m_loader->loadModule(location);
}

tempo_utils::Result<Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>>
zuri_test::PlaceholderLoader::loadPlugin(
    const lyric_common::ModuleLocation &location,
    const lyric_object::PluginSpecifier &specifier)
{
    if (m_loader == nullptr)
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant,
            "placeholder loader is not resolved");
    return m_loader->loadPlugin(location, specifier);
}
