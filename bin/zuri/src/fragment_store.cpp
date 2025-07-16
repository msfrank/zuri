/* SPDX-License-Identifier: AGPL-3.0-or-later */

#include <lyric_build/build_result.h>
#include <tempo_security/sha256_hash.h>
#include <tempo_utils/memory_bytes.h>
#include <tempo_utils/uuid.h>
#include <zuri/fragment_store.h>

FragmentStore::FragmentStore()
{
}

Option<bool>
FragmentStore::containsResource(const tempo_utils::Url &url)
{
    if (url.schemeView() != "x.fragment")
        return Option<bool>();
    return Option(m_meta.contains(url));
}

tempo_utils::Result<Option<lyric_build::Resource>>
FragmentStore::fetchResource(const tempo_utils::Url &url)
{
    if (url.schemeView() != "x.fragment")
        return Option<lyric_build::Resource>();
    if (!m_meta.contains(url))
        return Option<lyric_build::Resource>();
    return Option(m_meta.at(url));
}

tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>>
FragmentStore::loadResource(std::string_view resourceId)
{
    if (!m_content.contains(resourceId))
        return tempo_utils::GenericStatus::forCondition(
            tempo_utils::GenericCondition::kInternalViolation, "missing resource {}", resourceId);
    return m_content.at(resourceId);
}

tempo_utils::Result<lyric_build::ResourceList>
FragmentStore::listResources(
   const tempo_utils::Url &resourceRoot,
   lyric_build::ResourceMatcherFunc matcherFunc,
   const std::string &token)
{
    return tempo_utils::GenericStatus::forCondition(tempo_utils::GenericCondition::kUnimplemented,
        "listResources unimplemented");
}

tempo_utils::Result<lyric_build::ResourceList>
FragmentStore::listResourcesRecursively(
    const tempo_utils::Url &resourceRoot,
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
    return m_meta.contains(url);
}

tempo_utils::Result<Option<lyric_common::ModuleLocation>>
FragmentStore::resolveModule(const lyric_common::ModuleLocation &location) const
{
    auto url = location.toUrl();
    if (url.schemeView() != "x.fragment")
        return Option<lyric_common::ModuleLocation>();
    return Option(location);
}

tempo_utils::Result<Option<lyric_object::LyricObject>>
FragmentStore::loadModule(const lyric_common::ModuleLocation &location)
{
    if (!m_objects.contains(location))
        return Option<lyric_object::LyricObject>();
    return Option(m_objects.at(location));
}

tempo_utils::Result<Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>>
FragmentStore::loadPlugin(
    const lyric_common::ModuleLocation &location,
    const lyric_object::PluginSpecifier &specifier)
{
    return Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>();
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
    m_meta[fragmentUrl] = resource;

    m_content[resource.id] = tempo_utils::MemoryBytes::copy(fragment);

    return resource.id;
}

void
FragmentStore::insertObject(
    const lyric_common::ModuleLocation &assemblyLocation,
    const lyric_object::LyricObject &object)
{
    m_objects[assemblyLocation] = object;
}
