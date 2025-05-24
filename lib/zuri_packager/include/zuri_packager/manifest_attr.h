#ifndef ZURI_PACKAGER_MANIFEST_ATTR_H
#define ZURI_PACKAGER_MANIFEST_ATTR_H

#include <tempo_schema/attr.h>

#include "manifest_state.h"

namespace zuri_packager {

    class ManifestAttr {

    public:
        ManifestAttr(
            AttrId id,
            tempo_schema::AttrValue value,
            AttrAddress address,
            ManifestState *state);

        AttrId getAttrId() const;
        tempo_schema::AttrValue getAttrValue() const;
        AttrAddress getAddress() const;

    private:
        AttrId m_id;
        tempo_schema::AttrValue m_value;
        AttrAddress m_address;
        ManifestState *m_state;
    };
}

#endif // ZURI_PACKAGER_MANIFEST_ATTR_H