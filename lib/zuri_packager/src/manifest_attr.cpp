
#include <zuri_packager/manifest_attr.h>

zuri_packager::ManifestAttr::ManifestAttr(
    AttrId id,
    tempo_schema::AttrValue value,
    AttrAddress address,
    ManifestState *state)
    : m_id(id),
      m_value(value),
      m_address(address),
      m_state(state)
{
    TU_ASSERT (m_address.isValid());
    TU_ASSERT (m_state != nullptr);
}

zuri_packager::AttrId
zuri_packager::ManifestAttr::getAttrId() const
{
    return m_id;
}

tempo_schema::AttrValue
zuri_packager::ManifestAttr::getAttrValue() const
{
    return m_value;
}

zuri_packager::AttrAddress
zuri_packager::ManifestAttr::getAddress() const
{
    return m_address;
}