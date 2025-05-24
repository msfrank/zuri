#ifndef ZURI_PACKAGER_MANIFEST_ATTR_PARSER_H
#define ZURI_PACKAGER_MANIFEST_ATTR_PARSER_H

#include <tempo_schema/abstract_attr_parser.h>

#include "package_types.h"

namespace zuri_packager {

    class ManifestAttrParser : public tempo_schema::AbstractAttrParser {
    public:
        explicit ManifestAttrParser(std::shared_ptr<const internal::ManifestReader> reader);
        virtual tempo_utils::Status getNil(tu_uint32 index, std::nullptr_t &nil) override;
        virtual tempo_utils::Status getBool(tu_uint32 index, bool &b) override;
        virtual tempo_utils::Status getInt64(tu_uint32 index, tu_int64 &i64) override;
        virtual tempo_utils::Status getFloat64(tu_uint32 index, double &dbl) override;
        virtual tempo_utils::Status getUInt64(tu_uint32 index, tu_uint64 &u64) override;
        virtual tempo_utils::Status getUInt32(tu_uint32 index, tu_uint32 &u32) override;
        virtual tempo_utils::Status getUInt16(tu_uint32 index, tu_uint16 &u16) override;
        virtual tempo_utils::Status getUInt8(tu_uint32 index, tu_uint8 &u8) override;
        virtual tempo_utils::Status getString(tu_uint32 index, std::string &str) override;

    private:
        std::shared_ptr<const internal::ManifestReader> m_reader;
    };
}

#endif // ZURI_PACKAGER_MANIFEST_ATTR_PARSER_H