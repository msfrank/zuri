#ifndef ZURI_PACKAGER_PACKAGE_SPECIFIER_H
#define ZURI_PACKAGER_PACKAGE_SPECIFIER_H

#include <filesystem>
#include <string>

#include <absl/strings/string_view.h>

#include <tempo_utils/integer_types.h>
#include <tempo_utils/url.h>

namespace zuri_packager {

    class PackageSpecifier {

    public:
        PackageSpecifier();
        PackageSpecifier(
            std::string_view packageName,
            std::string_view packageDomain,
            tu_uint32 majorVersion,
            tu_uint32 minorVersion,
            tu_uint32 patchVersion);
        PackageSpecifier(const PackageSpecifier &other);

        bool isValid() const;

        std::string getPackageName() const;
        std::string getPackageDomain() const;
        tu_uint32 getMajorVersion() const;
        tu_uint32 getMinorVersion() const;
        tu_uint32 getPatchVersion() const;

        std::string toString() const;
        std::filesystem::path toFilesystemPath(const std::filesystem::path &base = {}) const;
        tempo_utils::Url toUrl() const;

        static PackageSpecifier fromAuthority(const tempo_utils::UrlAuthority &authority);
        static PackageSpecifier fromUrl(const tempo_utils::Url &uri);

    private:
        std::string m_packageName;
        std::string m_packageDomain;
        tu_uint32 m_majorVersion;
        tu_uint32 m_minorVersion;
        tu_uint32 m_patchVersion;
    };
}

#endif // ZURI_PACKAGER_PACKAGE_SPECIFIER_H