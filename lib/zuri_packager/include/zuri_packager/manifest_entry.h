#ifndef ZURI_PACKAGER_MANIFEST_ENTRY_H
#define ZURI_PACKAGER_MANIFEST_ENTRY_H

#include <filesystem>
#include <tempo_schema/attr_serde.h>

#include "manifest_attr_writer.h"
#include "manifest_state.h"
#include "package_types.h"

namespace zuri_packager {

    class ManifestEntry {

    public:
        ManifestEntry(
            EntryType type,
            const EntryPath &path,
            EntryAddress address,
            ManifestState *state);

        EntryType getEntryType() const;
        EntryPath getEntryPath() const;
        std::string getEntryName() const;
        EntryAddress getAddress() const;

        tu_uint32 getEntryOffset() const;
        void setEntryOffset(tu_uint32 offset);
        tu_uint32 getEntrySize() const;
        void setEntrySize(tu_uint32 size);
        EntryAddress getEntryDict() const;
        void setEntryDict(EntryAddress dict);
        EntryAddress getEntryLink() const;
        void setEntryLink(EntryAddress link);

        bool hasAttr(const AttrId &attrId) const;
        AttrAddress getAttr(const AttrId &attrId) const;
        PackageStatus putAttr(ManifestAttr *attr);
        absl::flat_hash_map<AttrId,AttrAddress>::const_iterator attrsBegin() const;
        absl::flat_hash_map<AttrId,AttrAddress>::const_iterator attrsEnd() const;
        int numAttrs() const;

        bool hasChild(std::string_view name) const;
        EntryAddress getChild(std::string_view name);
        PackageStatus putChild(ManifestEntry *child);
        absl::flat_hash_map<std::string,EntryAddress>::const_iterator childrenBegin() const;
        absl::flat_hash_map<std::string,EntryAddress>::const_iterator childrenEnd() const;
        int numChildren() const;

    private:
        EntryType m_type;
        EntryPath m_path;
        EntryAddress m_address;
        tu_uint32 m_offset;
        tu_uint32 m_size;
        EntryAddress m_dict;
        EntryAddress m_link;
        absl::flat_hash_map<AttrId,AttrAddress> m_attrs;
        absl::flat_hash_map<std::string,EntryAddress> m_children;
        ManifestState *m_state;

    public:
        /**
         *
         * @tparam T
         * @param serde
         * @param value
         * @return
         */
        template <typename T>
        tempo_utils::Status putAttr(const tempo_schema::AttrSerde<T> &serde, const T &value)
        {
            ManifestAttrWriter writer(serde.getKey(), &m_state);
            auto result = serde.writeAttr(&writer, value);
            if (result.isStatus())
                return result.getStatus();
            auto *attr = m_state->getAttr(result.getResult());
            TU_ASSERT (attr != nullptr);
            return putAttr(attr);
        };
    };
}

#endif // ZURI_PACKAGER_MANIFEST_ENTRY_H