#ifndef ZURI_PACKAGER_PACKAGE_WRITER_H
#define ZURI_PACKAGER_PACKAGE_WRITER_H

#include <filesystem>
#include <span>
#include <string>

#include <tempo_schema/attr_serde.h>
#include <tempo_utils/immutable_bytes.h>

#include "manifest_attr_writer.h"
#include "manifest_entry.h"
#include "manifest_state.h"
#include "package_result.h"

namespace zuri_packager {

    struct PackageWriterOptions {
        int maxLinkDepth = 5;
    };

    class PackageWriter {

    public:
        PackageWriter();
        PackageWriter(const PackageWriterOptions &options);

        tempo_utils::Status configure();

        bool hasEntry(const EntryPath &path) const;
        bool hasEntry(EntryAddress parentDirectory, std::string_view name) const;
        EntryAddress getEntry(const EntryPath &path) const;

        tempo_utils::Result<EntryAddress> makeDirectory(const EntryPath &path, bool createIntermediate = false);
        tempo_utils::Result<EntryAddress> makeDirectory(EntryAddress parentDirectory, std::string_view name);
        tempo_utils::Result<EntryAddress> putFile(EntryAddress parentDirectory,
            std::string_view name,
            std::shared_ptr<const tempo_utils::ImmutableBytes> bytes);
        tempo_utils::Result<EntryAddress> putFile(
            const EntryPath &path,
            std::shared_ptr<const tempo_utils::ImmutableBytes> bytes);
        tempo_utils::Result<EntryAddress> linkToTarget(const EntryPath &path, EntryAddress target);

        tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>> toBytes() const;

    private:
        PackageWriterOptions m_options;
        ManifestState m_state;
        ManifestEntry *m_packageEntry;
        absl::flat_hash_map<EntryPath,std::shared_ptr<const tempo_utils::ImmutableBytes>> m_contents;

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
            if (m_packageEntry == nullptr)
                return PackageStatus::forCondition(
                    PackageCondition::kPackageInvariant, "writer is not configured");
            ManifestAttrWriter writer(serde.getKey(), &m_state);
            auto result = serde.writeAttr(&writer, value);
            if (result.isStatus())
                return result.getStatus();
            auto *attr = m_state.getAttr(result.getResult());
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
            auto *entry = m_state.getEntry(address.getAddress());
            if (entry == nullptr)
                return PackageStatus::forCondition(
                    PackageCondition::kPackageInvariant, "invalid entry address");

            ManifestAttrWriter writer(serde.getKey(), &m_state);
            auto result = serde.writeAttr(&writer, value);
            if (result.isStatus())
                return result.getStatus();
            auto *attr = m_state.getAttr(result.getResult());
            if (attr == nullptr)
                return PackageStatus::forCondition(
                    PackageCondition::kPackageInvariant, "missing serialized attr");
            return entry->putAttr(attr);
        };
    };
}

#endif // ZURI_PACKAGER_PACKAGE_WRITER_H
