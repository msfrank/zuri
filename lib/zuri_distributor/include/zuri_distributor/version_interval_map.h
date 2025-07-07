#ifndef ZURI_DISTRIBUTOR_VERSION_INTERVAL_MAP_H
#define ZURI_DISTRIBUTOR_VERSION_INTERVAL_MAP_H

#include <tempo_utils/status.h>
#include <zuri_packager/package_dependency.h>

namespace zuri_distributor {

    class VersionIntervalMap {
    public:
        VersionIntervalMap() = default;

        tempo_utils::Status putDependency();

    private:
        struct Priv;
        std::unique_ptr<Priv> m_priv;
    };
}

#endif // ZURI_DISTRIBUTOR_VERSION_INTERVAL_MAP_H
