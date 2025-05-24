
#include <zuri_packager/manifest_namespace.h>

zuri_packager::ManifestNamespace::ManifestNamespace(
    const tempo_utils::Url &nsUrl,
    NamespaceAddress address,
    ManifestState *state)
    : m_nsUrl(nsUrl),
      m_address(address),
      m_state(state)
{
    TU_ASSERT (m_nsUrl.isValid());
    TU_ASSERT (m_address.isValid());
    TU_ASSERT (m_state != nullptr);
}

tempo_utils::Url
zuri_packager::ManifestNamespace::getNsUrl() const
{
    return m_nsUrl;
}

zuri_packager::NamespaceAddress
zuri_packager::ManifestNamespace::getAddress() const
{
    return m_address;
}
