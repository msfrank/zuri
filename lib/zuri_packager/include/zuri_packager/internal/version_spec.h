#ifndef ZURI_PACKAGER_INTERNAL_VERSION_SPEC_H
#define ZURI_PACKAGER_INTERNAL_VERSION_SPEC_H

#include <tempo_utils/integer_types.h>
#include <tempo_utils/option_template.h>

#include "../package_types.h"

namespace zuri_packager::internal {

    enum class VersionSpecType {
        Invalid,
        FullVersion,
        ApiVersion,
        MajorVersion,
        AnyVersion,
    };

    class VersionSpec {
    public:
        VersionSpec();
        explicit VersionSpec(tu_uint32 majorVersion);
        VersionSpec(tu_uint32 majorVersion, tu_uint32 minorVersion);
        VersionSpec(tu_uint32 majorVersion, tu_uint32 minorVersion, tu_uint32 patchVersion);
        VersionSpec(const VersionSpec &other);

        VersionSpecType getType() const;

        bool hasMajorVersion() const;
        tu_uint32 getMajorVersion() const;

        bool hasMinorVersion() const;
        tu_uint32 getMinorVersion() const;

        bool hasPatchVersion() const;
        tu_uint32 getPatchVersion() const;

        PackageVersion toVersion() const;

    private:
        struct Priv {
            VersionSpecType type;
            Option<tu_uint32> majorVersion;
            Option<tu_uint32> minorVersion;
            Option<tu_uint32> patchVersion;
        };
        std::shared_ptr<Priv> m_priv;
    };
}

#endif // ZURI_PACKAGER_INTERNAL_VERSION_SPEC_H
