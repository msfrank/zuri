#ifndef ZURI_PACKAGER_PACKAGE_WRITER_H
#define ZURI_PACKAGER_PACKAGE_WRITER_H

#include <filesystem>
#include <span>
#include <string>
#include <tempo_config/config_types.h>

#include <tempo_schema/attr_serde.h>
#include <tempo_utils/immutable_bytes.h>

#include "manifest_attr_writer.h"
#include "manifest_entry.h"
#include "manifest_state.h"
#include "package_result.h"
#include "package_specifier.h"

namespace zuri_packager {

    constexpr std::filesystem::perms kDefaultDirectoryPerms =
        std::filesystem::perms::owner_all
        | std::filesystem::perms::group_read
        | std::filesystem::perms::group_exec
        | std::filesystem::perms::others_read
        | std::filesystem::perms::others_exec
        ;

    struct PackageWriterOptions {
        /**
         * the directory where the package will be written to. by default the package is written
         * to the current working directory.
         */
        std::filesystem::path installRoot = {};
        /**
         *
         */
        int maxLinkDepth = 5;
        /**
         * if true, then the package config will not be written to the package, and must be added
         * to the package manually via putFile.
         */
        bool skipPackageConfig = false;
    };

    class PackageWriter {

    public:
        explicit PackageWriter(const PackageSpecifier &specifier, const PackageWriterOptions &options = {});

        tempo_utils::Status configure();

        bool hasEntry(const tempo_utils::UrlPath &path) const;
        bool hasEntry(EntryAddress parentDirectory, std::string_view name) const;
        EntryAddress getEntry(const tempo_utils::UrlPath &path) const;

        tempo_utils::Result<EntryAddress> makeDirectory(
            const tempo_utils::UrlPath &path,
            bool createIntermediate = false);
        tempo_utils::Result<EntryAddress> makeDirectory(
            EntryAddress parentDirectory,
            std::string_view name);
        tempo_utils::Result<EntryAddress> putFile(
            EntryAddress parentDirectory,
            std::string_view name,
            std::shared_ptr<const tempo_utils::ImmutableBytes> bytes);
        tempo_utils::Result<EntryAddress> putFile(
            const tempo_utils::UrlPath &path,
            std::shared_ptr<const tempo_utils::ImmutableBytes> bytes);
        tempo_utils::Result<EntryAddress> linkToTarget(
            const tempo_utils::UrlPath &path,
            EntryAddress target);

        tempo_config::ConfigMap getPackageConfig() const;
        void setPackageConfig(const tempo_config::ConfigMap &packageConfig);

        tempo_utils::Result<std::filesystem::path> writePackage();

    private:
        PackageSpecifier m_specifier;
        PackageWriterOptions m_options;

        std::unique_ptr<ManifestState> m_state;
        ManifestEntry *m_packageEntry;
        absl::flat_hash_map<
            tempo_utils::UrlPath,
            std::shared_ptr<const tempo_utils::ImmutableBytes>> m_contents;
        tempo_config::ConfigMap m_config;

    public:
        /**
         *
         * @tparam T
         * @param serde
         * @param value
         * @return
         */
        template <typename T>
        tempo_utils::Status putPackageAttr(const tempo_schema::AttrSerde<T> &serde, const T &value)
        {
            if (m_state == nullptr)
                return PackageStatus::forCondition(
                    PackageCondition::kPackageInvariant, "writer is finished");
            if (m_packageEntry == nullptr)
                return PackageStatus::forCondition(
                    PackageCondition::kPackageInvariant, "writer is not configured");
            ManifestAttrWriter writer(serde.getKey(), &m_state);
            auto result = serde.writeAttr(&writer, value);
            if (result.isStatus())
                return result.getStatus();
            auto *attr = m_state->getAttr(result.getResult());
            if (attr == nullptr)
                return PackageStatus::forCondition(
                    PackageCondition::kPackageInvariant, "missing serialized attr");
            return m_packageEntry->putAttr(attr);
        };
        /**
         *
         * @tparam T
         * @param serde
         * @param value
         * @return
         */
        template <typename T>
        tempo_utils::Status putEntryAttr(
            EntryAddress address,
            const tempo_schema::AttrSerde<T> &serde,
            const T &value)
        {
            if (!address.isValid())
                return PackageStatus::forCondition(
                    PackageCondition::kPackageInvariant, "invalid entry address");
            if (m_state == nullptr)
                return PackageStatus::forCondition(
                    PackageCondition::kPackageInvariant, "writer is finished");
            if (m_packageEntry == nullptr)
                return PackageStatus::forCondition(
                    PackageCondition::kPackageInvariant, "writer is not configured");
            auto *entry = m_state->getEntry(address.getAddress());
            if (entry == nullptr)
                return PackageStatus::forCondition(
                    PackageCondition::kPackageInvariant, "invalid entry address");

            ManifestAttrWriter writer(serde.getKey(), &m_state);
            auto result = serde.writeAttr(&writer, value);
            if (result.isStatus())
                return result.getStatus();
            auto *attr = m_state->getAttr(result.getResult());
            if (attr == nullptr)
                return PackageStatus::forCondition(
                    PackageCondition::kPackageInvariant, "missing serialized attr");
            return entry->putAttr(attr);
        };
    };
}

#endif // ZURI_PACKAGER_PACKAGE_WRITER_H
