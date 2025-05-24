#ifndef ZURI_PACKAGER_MANIFEST_ATTR_WRITER_H
#define ZURI_PACKAGER_MANIFEST_ATTR_WRITER_H

#include <tempo_schema/abstract_attr_writer.h>

#include "manifest_state.h"

namespace zuri_packager {

    class ManifestAttrWriter : public tempo_schema::AbstractAttrWriter {
    public:
        ManifestAttrWriter(const tempo_schema::AttrKey &key, ManifestState *state);
        tempo_utils::Result<tu_uint32> putNamespace(const tempo_utils::Url &nsUrl) override;
        tempo_utils::Result<tu_uint32> putNil() override;
        tempo_utils::Result<tu_uint32> putBool(bool b) override;
        tempo_utils::Result<tu_uint32> putInt64(tu_int64 i64) override;
        tempo_utils::Result<tu_uint32> putFloat64(double dbl) override;
        tempo_utils::Result<tu_uint32> putUInt64(tu_uint64 u64) override;
        tempo_utils::Result<tu_uint32> putUInt32(tu_uint32 u32) override;
        tempo_utils::Result<tu_uint32> putUInt16(tu_uint16 u16) override;
        tempo_utils::Result<tu_uint32> putUInt8(tu_uint8 u8) override;
        tempo_utils::Result<tu_uint32> putString(std::string_view str) override;

    private:
        tempo_schema::AttrKey m_key;
        ManifestState *m_state;
    };
}

#endif // ZURI_PACKAGER_MANIFEST_ATTR_WRITER_H