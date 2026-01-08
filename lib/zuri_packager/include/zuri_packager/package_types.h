#ifndef ZURI_PACKAGER_PACKAGE_TYPES_H
#define ZURI_PACKAGER_PACKAGE_TYPES_H

#include <memory>
#include <string>

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <tempo_utils/integer_types.h>
#include <tempo_utils/url_authority.h>

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
        tu_uint32 bytecodeOffset;        // absolute offset to the object bytecode
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

    class PackageId {
    public:
        PackageId();
        PackageId(std::string_view name, std::string_view domain);
        PackageId(const PackageId &other);

        bool isValid() const;

        std::string getName() const;
        std::string getDomain() const;

        std::string toString() const;

        int compare(const PackageId &other) const;

        bool operator==(const PackageId &other) const;
        bool operator!=(const PackageId &other) const;
        bool operator<=(const PackageId &other) const;
        bool operator<(const PackageId &other) const;
        bool operator>=(const PackageId &other) const;
        bool operator>(const PackageId &other) const;

        static PackageId fromString(std::string_view s);
        static PackageId fromAuthority(const tempo_utils::UrlAuthority &authority);

        template <typename H>
        friend H AbslHashValue(H h, const PackageId &id) {
            if (id.isValid())
                return H::combine(std::move(h), id.getName(), id.getDomain());
            return H::combine(std::move(h), 0);
        }

    private:
        struct Priv {
            std::string name;
            std::string domain;
        };
        std::shared_ptr<Priv> m_priv;
    };

    class PackageVersion {
    public:
        PackageVersion();
        PackageVersion(tu_uint32 majorVersion, tu_uint32 minorVersion, tu_uint32 patchVersion);
        PackageVersion(const PackageVersion &other);

        bool isValid() const;

        tu_uint32 getMajorVersion() const;
        tu_uint32 getMinorVersion() const;
        tu_uint32 getPatchVersion() const;

        std::string toString() const;

        int compare(const PackageVersion &other) const;

        bool operator==(const PackageVersion &other) const;
        bool operator!=(const PackageVersion &other) const;
        bool operator<=(const PackageVersion &other) const;
        bool operator<(const PackageVersion &other) const;
        bool operator>=(const PackageVersion &other) const;
        bool operator>(const PackageVersion &other) const;

        static PackageVersion fromString(std::string_view s);

        template <typename H>
        friend H AbslHashValue(H h, const PackageVersion &version) {
            if (version.isValid())
                return H::combine(std::move(h), version.getMajorVersion(),
                    version.getMinorVersion(), version.getPatchVersion());
            return H::combine(std::move(h), 0);
        }

    private:
        struct Priv {
            tu_uint32 majorVersion;
            tu_uint32 minorVersion;
            tu_uint32 patchVersion;
        };
        std::shared_ptr<Priv> m_priv;
    };

    class RequirementsMap {
    public:
        RequirementsMap();
        explicit RequirementsMap(const absl::flat_hash_map<PackageId,PackageVersion> &requirements);
        RequirementsMap(const RequirementsMap &other);

        absl::flat_hash_map<PackageId,PackageVersion>::const_iterator requirementsBegin() const;
        absl::flat_hash_map<PackageId,PackageVersion>::const_iterator requirementsEnd() const;
        int numRequirements() const;

    private:
        std::shared_ptr<absl::flat_hash_map<PackageId,PackageVersion>> m_requirements;
    };

    class LibrariesNeeded {
    public:
        LibrariesNeeded();
        LibrariesNeeded(const LibrariesNeeded &other);

        void addSystemLibrary(std::string_view libraryName);
        void addSystemLibraries(const absl::flat_hash_set<std::string> &libraryNames);
        absl::flat_hash_set<std::string>::const_iterator systemLibrariesBegin() const;
        absl::flat_hash_set<std::string>::const_iterator systemLibrariesEnd() const;

        void addDistributionLibrary(std::string_view libraryName);
        void addDistributionLibraries(const absl::flat_hash_set<std::string> &libraryNames);
        absl::flat_hash_set<std::string>::const_iterator distributionLibrariesBegin() const;
        absl::flat_hash_set<std::string>::const_iterator distributionLibrariesEnd() const;

        void addPackageLibrary(const PackageId &packageId, std::string_view libraryName);
        void addPackageLibraries(const PackageId &packageId, const absl::flat_hash_set<std::string> &libraryNames);
        absl::flat_hash_set<PackageId>::const_iterator packagesBegin() const;
        absl::flat_hash_set<PackageId>::const_iterator packagesEnd() const;
        absl::flat_hash_set<std::string>::const_iterator packageLibrariesBegin(const PackageId &packageId) const;
        absl::flat_hash_set<std::string>::const_iterator packageLibrariesEnd(const PackageId &packageId) const;

    private:
        struct Priv {
            absl::flat_hash_set<std::string> systemLibraries;
            absl::flat_hash_set<std::string> distributionLibraries;
            absl::flat_hash_set<PackageId> packagesNeeded;
            absl::flat_hash_map<
                PackageId,
                std::unique_ptr<absl::flat_hash_set<std::string>>
            > packageLibraries;
        };
        std::shared_ptr<Priv> m_priv;
    };

    class LibrariesProvided {
    public:
        LibrariesProvided();
        LibrariesProvided(const LibrariesProvided &other);

        void addLibrary(std::string_view libraryName);
        void addLibraries(const absl::flat_hash_set<std::string> &libraryNames);
        absl::flat_hash_set<std::string>::const_iterator providedBegin() const;
        absl::flat_hash_set<std::string>::const_iterator providedEnd() const;

    private:
        struct Priv {
            absl::flat_hash_set<std::string> provided;
        };
        std::shared_ptr<Priv> m_priv;
    };

    // forward declarations
    namespace internal {
        class ManifestReader;
    }
}

#endif // ZURI_PACKAGER_PACKAGE_TYPES_H
