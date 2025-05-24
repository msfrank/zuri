
#include <tempo_schema/schema_result.h>
#include <zuri_packager/generated/manifest.h>
#include <zuri_packager/internal/manifest_reader.h>
#include <zuri_packager/manifest_attr_parser.h>

zuri_packager::ManifestAttrParser::ManifestAttrParser(std::shared_ptr<const internal::ManifestReader> reader)
    : m_reader(reader)
{
    TU_ASSERT (m_reader != nullptr);
}

tempo_utils::Status
zuri_packager::ManifestAttrParser::getNil(tu_uint32 index, std::nullptr_t &nil)
{
    if (!m_reader->isValid())
        return tempo_schema::SchemaStatus::forCondition(tempo_schema::SchemaCondition::kParseError, "invalid reader");
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->attr_value_type() != zpk1::Value::TrueFalseNilValue)
        return tempo_schema::SchemaStatus::forCondition(tempo_schema::SchemaCondition::kWrongType,"attr type mismatch");
    auto *value = attr->attr_value_as_TrueFalseNilValue();
    if (value->tfn() != zpk1::TrueFalseNil::Nil)
        return tempo_schema::SchemaStatus::forCondition(tempo_schema::SchemaCondition::kWrongType,"attr type mismatch");
    nil = nullptr;
    return {};
}

tempo_utils::Status
zuri_packager::ManifestAttrParser::getBool(tu_uint32 index, bool &b)
{
    if (!m_reader->isValid())
        return tempo_schema::SchemaStatus::forCondition(tempo_schema::SchemaCondition::kParseError, "invalid reader");
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->attr_value_type() != zpk1::Value::TrueFalseNilValue)
        return tempo_schema::SchemaStatus::forCondition(tempo_schema::SchemaCondition::kWrongType,"attr type mismatch");
    auto *value = attr->attr_value_as_TrueFalseNilValue();
    switch (value->tfn()) {
        case zpk1::TrueFalseNil::True:
            b = true;
            return {};
        case zpk1::TrueFalseNil::False:
            b = false;
            return {};
        case zpk1::TrueFalseNil::Nil:
        default:
            return tempo_schema::SchemaStatus::forCondition(tempo_schema::SchemaCondition::kWrongType,"attr type mismatch");
    }
}

tempo_utils::Status
zuri_packager::ManifestAttrParser::getInt64(tu_uint32 index, tu_int64 &i64)
{
    if (!m_reader->isValid())
        return tempo_schema::SchemaStatus::forCondition(tempo_schema::SchemaCondition::kParseError, "invalid reader");
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->attr_value_type() != zpk1::Value::Int64Value)
        return tempo_schema::SchemaStatus::forCondition(tempo_schema::SchemaCondition::kWrongType,"attr type mismatch");
    auto *value = attr->attr_value_as_Int64Value();
    i64 = value->i64();
    return {};
}

tempo_utils::Status
zuri_packager::ManifestAttrParser::getFloat64(tu_uint32 index, double &dbl)
{
    if (!m_reader->isValid())
        return tempo_schema::SchemaStatus::forCondition(tempo_schema::SchemaCondition::kParseError, "invalid reader");
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->attr_value_type() != zpk1::Value::Float64Value)
        return tempo_schema::SchemaStatus::forCondition(tempo_schema::SchemaCondition::kWrongType,"attr type mismatch");
    auto *value = attr->attr_value_as_Float64Value();
    dbl = value->f64();
    return {};
}

tempo_utils::Status
zuri_packager::ManifestAttrParser::getUInt64(tu_uint32 index, tu_uint64 &u64)
{
    if (!m_reader->isValid())
        return tempo_schema::SchemaStatus::forCondition(tempo_schema::SchemaCondition::kParseError, "invalid reader");
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->attr_value_type() != zpk1::Value::UInt64Value)
        return tempo_schema::SchemaStatus::forCondition(tempo_schema::SchemaCondition::kWrongType,"attr type mismatch");
    auto *value = attr->attr_value_as_UInt64Value();
    u64 = value->u64();
    return {};
}

tempo_utils::Status
zuri_packager::ManifestAttrParser::getUInt32(tu_uint32 index, tu_uint32 &u32)
{
    if (!m_reader->isValid())
        return tempo_schema::SchemaStatus::forCondition(tempo_schema::SchemaCondition::kParseError, "invalid reader");
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->attr_value_type() != zpk1::Value::UInt32Value)
        return tempo_schema::SchemaStatus::forCondition(tempo_schema::SchemaCondition::kWrongType,"attr type mismatch");
    auto *value = attr->attr_value_as_UInt32Value();
    u32 = value->u32();
    return {};
}

tempo_utils::Status
zuri_packager::ManifestAttrParser::getUInt16(tu_uint32 index, tu_uint16 &u16)
{
    if (!m_reader->isValid())
        return tempo_schema::SchemaStatus::forCondition(tempo_schema::SchemaCondition::kParseError, "invalid reader");
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->attr_value_type() != zpk1::Value::UInt16Value)
        return tempo_schema::SchemaStatus::forCondition(tempo_schema::SchemaCondition::kWrongType,"attr type mismatch");
    auto *value = attr->attr_value_as_UInt16Value();
    u16 = value->u16();
    return {};
}

tempo_utils::Status
zuri_packager::ManifestAttrParser::getUInt8(tu_uint32 index, tu_uint8 &u8)
{
    if (!m_reader->isValid())
        return tempo_schema::SchemaStatus::forCondition(tempo_schema::SchemaCondition::kParseError, "invalid reader");
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->attr_value_type() != zpk1::Value::UInt8Value)
        return tempo_schema::SchemaStatus::forCondition(tempo_schema::SchemaCondition::kWrongType,"attr type mismatch");
    auto *value = attr->attr_value_as_UInt8Value();
    u8 = value->u8();
    return {};
}

tempo_utils::Status
zuri_packager::ManifestAttrParser::getString(tu_uint32 index, std::string &str)
{
    if (!m_reader->isValid())
        return tempo_schema::SchemaStatus::forCondition(tempo_schema::SchemaCondition::kParseError, "invalid reader");
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->attr_value_type() != zpk1::Value::StringValue)
        return tempo_schema::SchemaStatus::forCondition(tempo_schema::SchemaCondition::kWrongType,"attr type mismatch");
    auto *value = attr->attr_value_as_StringValue();
    str = value->utf8()? value->utf8()->str() : std::string();
    return {};
}
