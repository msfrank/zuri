#ifndef ZURI_PACKAGER_PACKAGE_TYPES_H
#define ZURI_PACKAGER_PACKAGE_TYPES_H

#include <memory>

#include <tempo_utils/integer_types.h>

#define ZURI_PACKAGER_PACKAGE_FILE_SUFFIX                "zpk"
#define ZURI_PACKAGER_PACKAGE_FILE_DOT_SUFFIX            ".zpk"
#define ZURI_PACKAGER_PACKAGE_CONTENT_TYPE               "application/vnd.zuri.package"

namespace zuri_packager {

    constexpr tu_uint32 kInvalidOffsetU32       = 0xffffffff;
    constexpr tu_uint16 kInvalidOffsetU16       = 0xffff;
    constexpr tu_uint8 kInvalidOffsetU8         = 0xff;

    constexpr int kMaximumLinkRecursion         = 5;

    constexpr const char *kPackageFileSuffix    = ZURI_PACKAGER_PACKAGE_FILE_SUFFIX;
    constexpr const char *kPackageFileDotSuffix = ZURI_PACKAGER_PACKAGE_FILE_DOT_SUFFIX;
    constexpr const char *PackagetContentType   = ZURI_PACKAGER_PACKAGE_CONTENT_TYPE;

    enum class ManifestVersion {
        Unknown,
        Version1,
    };

    enum class EntryType {
        Invalid,
        File,
        Directory,
        Link,
        Package,
    };

    struct PrologueEntry {
        tu_uint8 version;                // prologue version, should be 1
        tu_uint32 flags;                 // prologue flags, should be 0
        tu_uint32 bytecodeOffset;        // absolute offset to the assembly bytecode
        tu_uint32 sectionSize;           // size of the section (prologue + bytecode + linkage table) in bytes
    };

    struct NamespaceAddress {
    public:
        NamespaceAddress() : u32(kInvalidOffsetU32) {};
        explicit NamespaceAddress(tu_uint32 u32) : u32(u32) {};
        NamespaceAddress(const NamespaceAddress &other) : u32(other.u32) {};
        bool isValid() const { return u32 != kInvalidOffsetU32; }
        tu_uint32 getAddress() const { return u32; };
        bool operator==(const NamespaceAddress &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32 = kInvalidOffsetU32;
    };

    struct AttrAddress {
    public:
        AttrAddress() : u32(kInvalidOffsetU32) {};
        explicit AttrAddress(tu_uint32 u32) : u32(u32) {};
        AttrAddress(const AttrAddress &other) : u32(other.u32) {};
        bool isValid() const { return u32 != kInvalidOffsetU32; }
        tu_uint32 getAddress() const { return u32; };
        bool operator==(const AttrAddress &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32 = kInvalidOffsetU32;
    };

    struct EntryAddress {
    public:
        EntryAddress() : u32(kInvalidOffsetU32) {};
        explicit EntryAddress(tu_uint32 u32) : u32(u32) {};
        EntryAddress(const EntryAddress &other) : u32(other.u32) {};
        bool isValid() const { return u32 != kInvalidOffsetU32; }
        tu_uint32 getAddress() const { return u32; };
        bool operator==(const EntryAddress &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32 = kInvalidOffsetU32;
    };

    struct AttrId {
        AttrId();
        AttrId(const NamespaceAddress &address, tu_uint32 type);
        AttrId(const AttrId &other);

        NamespaceAddress getAddress() const;
        tu_uint32 getType() const;

        bool operator==(const AttrId &other) const;

        template <typename H>
        friend H AbslHashValue(H h, const AttrId &id) {
            return H::combine(std::move(h), id.m_address.getAddress(), id.m_type);
        }

    private:
        NamespaceAddress m_address;
        tu_uint32 m_type;
    };

    // forward declarations
    namespace internal {
        class ManifestReader;
    }
}

#endif // ZURI_PACKAGER_PACKAGE_TYPES_H
