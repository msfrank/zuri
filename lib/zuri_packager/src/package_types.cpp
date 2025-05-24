
#include <zuri_packager/package_types.h>
#include <tempo_utils/log_message.h>

zuri_packager::AttrId::AttrId()
    : m_address(),
      m_type(kInvalidOffsetU32)
{
}

zuri_packager::AttrId::AttrId(const NamespaceAddress &address, tu_uint32 type)
    : m_address(address),
      m_type(type)
{
    TU_ASSERT (m_address.isValid());
    TU_ASSERT (m_type != kInvalidOffsetU32);
}

zuri_packager::AttrId::AttrId(const AttrId &other)
    : m_address(other.m_address),
      m_type(other.m_type)
{
}

zuri_packager::NamespaceAddress
zuri_packager::AttrId::getAddress() const
{
    return m_address;
}

tu_uint32
zuri_packager::AttrId::getType() const
{
    return m_type;
}

bool
zuri_packager::AttrId::operator==(const AttrId &other) const
{
    return m_address == other.m_address && m_type == other.m_type;
}
