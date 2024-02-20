/* SPDX-License-Identifier: AGPL-3.0-or-later */

#ifndef ZURI_FRAGMENT_STORE_H
#define ZURI_FRAGMENT_STORE_H

#include <lyric_build/abstract_filesystem.h>
#include <lyric_runtime/abstract_loader.h>

class FragmentStore : public lyric_build::AbstractFilesystem, public lyric_runtime::AbstractLoader {
public:
    FragmentStore();

    Option<bool> containsResource(const tempo_utils::Url &url) override;
    tempo_utils::Result<Option<lyric_build::Resource>> fetchResource(const tempo_utils::Url &url) override;
    tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>> loadResource(
        std::string_view resourceId) override;
    tempo_utils::Result<lyric_build::ResourceList> listResources(
        const tempo_utils::Url &resourceRoot,
        lyric_build::ResourceMatcherFunc matcherFunc,
        const std::string &token) override;
    tempo_utils::Result<lyric_build::ResourceList> listResourcesRecursively(
        const tempo_utils::Url &resourceRoot,
        lyric_build::ResourceMatcherFunc matcherFunc,
        const std::string &token) override;

    tempo_utils::Result<bool> hasAssembly(
        const lyric_common::AssemblyLocation &location) const override;
    tempo_utils::Result<Option<lyric_common::AssemblyLocation>> resolveAssembly(
        const lyric_common::AssemblyLocation &location) const override;
    tempo_utils::Result<Option<lyric_object::LyricObject>> loadAssembly(
        const lyric_common::AssemblyLocation &location) override;
    tempo_utils::Result<Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>> loadPlugin(
        const lyric_common::AssemblyLocation &location,
        const lyric_object::PluginSpecifier &specifier) override;

    std::string insertFragment(
        const tempo_utils::Url &fragmentUrl,
        std::string_view fragment,
        tu_uint64 lastModifiedMillis);
    void insertAssembly(
        const lyric_common::AssemblyLocation &assemblyLocation,
        const lyric_object::LyricObject &object);

private:
    absl::flat_hash_map<tempo_utils::Url,lyric_build::Resource> m_meta;
    absl::flat_hash_map<
        std::string,
        std::shared_ptr<const tempo_utils::ImmutableBytes>> m_content;
    absl::flat_hash_map<
        lyric_common::AssemblyLocation,
        lyric_object::LyricObject> m_assemblies;
};

#endif //ZURI_FRAGMENT_STORE_H
