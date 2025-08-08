/* SPDX-License-Identifier: AGPL-3.0-or-later */

#include <lyric_build/build_result.h>
#include <tempo_security/sha256_hash.h>
#include <tempo_utils/memory_bytes.h>
#include <tempo_utils/uuid.h>
#include <zuri/fragment_store.h>

FragmentStore::FragmentStore()
{
}

bool
FragmentStore::containsResource(const tempo_utils::UrlPath &urlPath)
{
    return m_meta.contains(urlPath);
}

tempo_utils::Result<Option<lyric_build::Resource>>
FragmentStore::fetchResource(const tempo_utils::UrlPath &urlPath)
{
    auto entry = m_meta.find(urlPath);
    if (entry != m_meta.cend())
        return Option(entry->second);
    return {};
}

tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>>
FragmentStore::loadResource(std::string_view resourceId)
{
    auto entry = m_content.find(resourceId);
    if (entry == m_content.cend())
        return tempo_utils::GenericStatus::forCondition(
            tempo_utils::GenericCondition::kInternalViolation, "missing resource {}", resourceId);
    return entry->second;
}

tempo_utils::Result<lyric_build::ResourceList>
FragmentStore::listResources(
   const tempo_utils::UrlPath &resourceRoot,
   lyric_build::ResourceMatcherFunc matcherFunc,
   const std::string &token)
{
    return tempo_utils::GenericStatus::forCondition(tempo_utils::GenericCondition::kUnimplemented,
        "listResources unimplemented");
}

tempo_utils::Result<lyric_build::ResourceList>
FragmentStore::listResourcesRecursively(
    const tempo_utils::UrlPath &resourceRoot,
    lyric_build::ResourceMatcherFunc matcherFunc,
    const std::string &token)
{
    return tempo_utils::GenericStatus::forCondition(tempo_utils::GenericCondition::kUnimplemented,
        "listResourcesRecursively unimplemented");
}

tempo_utils::Result<bool>
FragmentStore::hasModule(const lyric_common::ModuleLocation &location) const
{
    auto url = location.toUrl();
    if (url.schemeView() != "x.fragment")
        return false;
    return m_meta.contains(url.toPath());
}

tempo_utils::Result<Option<lyric_object::LyricObject>>
FragmentStore::loadModule(const lyric_common::ModuleLocation &location)
{
    auto entry = m_objects.find(location);
    if (entry != m_objects.cend())
        return Option(entry->second);
    return {};
}

tempo_utils::Result<Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>>
FragmentStore::loadPlugin(
    const lyric_common::ModuleLocation &location,
    const lyric_object::PluginSpecifier &specifier)
{
    return {};
}

std::string
FragmentStore::insertFragment(
    const tempo_utils::Url &fragmentUrl,
    std::string_view fragment,
    tu_uint64 lastModifiedMillis)
{
    lyric_build::Resource resource;
    resource.id = tempo_utils::UUID::randomUUID().toString();
    resource.entityTag = tempo_security::Sha256Hash::hash(fragment);
    resource.lastModifiedMillis = lastModifiedMillis;

    auto fragmentPath = fragmentUrl.toPath();
    m_meta[fragmentPath] = resource;

    m_content[resource.id] = tempo_utils::MemoryBytes::copy(fragment);

    return resource.id;
}

void
FragmentStore::insertObject(
    const lyric_common::ModuleLocation &location,
    const lyric_object::LyricObject &object)
{
    m_objects[location] = object;
}
