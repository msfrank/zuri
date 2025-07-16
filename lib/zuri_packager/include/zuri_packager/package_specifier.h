#ifndef ZURI_PACKAGER_PACKAGE_SPECIFIER_H
#define ZURI_PACKAGER_PACKAGE_SPECIFIER_H

#include <filesystem>
#include <string>

#include <tempo_utils/integer_types.h>
#include <tempo_utils/url.h>

#include "package_types.h"

namespace zuri_packager {

    class PackageSpecifier {

    public:
        PackageSpecifier();
        PackageSpecifier(
            const PackageId &packageId,
            const PackageVersion &packageVersion);
        PackageSpecifier(
            const std::string &packageName,
            const std::string &packageDomain,
            tu_uint32 majorVersion,
            tu_uint32 minorVersion,
            tu_uint32 patchVersion);
        PackageSpecifier(const PackageSpecifier &other);

        bool isValid() const;

        PackageId getPackageId() const;
        PackageVersion getPackageVersion() const;

        std::string getPackageName() const;
        std::string getPackageDomain() const;
        tu_uint32 getMajorVersion() const;
        tu_uint32 getMinorVersion() const;
        tu_uint32 getPatchVersion() const;

        std::string getVersionString() const;

        int compare(const PackageSpecifier &other) const;

        bool operator==(const PackageSpecifier &other) const;
        bool operator!=(const PackageSpecifier &other) const;
        bool operator<=(const PackageSpecifier &other) const;
        bool operator<(const PackageSpecifier &other) const;
        bool operator>=(const PackageSpecifier &other) const;
        bool operator>(const PackageSpecifier &other) const;

        std::string toString() const;
        std::filesystem::path toFilesystemPath(const std::filesystem::path &base = {}) const;
        tempo_utils::Url toUrl() const;

        static PackageSpecifier fromString(const std::string &s);
        static PackageSpecifier fromAuthority(const tempo_utils::UrlAuthority &authority);
        static PackageSpecifier fromUrl(const tempo_utils::Url &url);
        static PackageSpecifier fromFilesystemName(const std::filesystem::path &name);

        template <typename H>
        friend H AbslHashValue(H h, const PackageSpecifier &specifier) {
            if (specifier.isValid())
                return H::combine(std::move(h), specifier.getPackageId(), specifier.getPackageVersion());
            return H::combine(std::move(h), 0);
        }

    private:
        struct Priv {
            PackageId id;
            PackageVersion version;
        };
        std::shared_ptr<Priv> m_priv;
    };
}

#endif // ZURI_PACKAGER_PACKAGE_SPECIFIER_H