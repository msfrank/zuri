
#include <flatbuffers/flatbuffers.h>

#include <zuri_packager/generated/manifest.h>
#include <zuri_packager/internal/manifest_reader.h>
#include <zuri_packager/zuri_manifest.h>

zuri_packager::ZuriManifest::ZuriManifest()
{
}

zuri_packager::ZuriManifest::ZuriManifest(std::shared_ptr<const tempo_utils::ImmutableBytes> immutableBytes)
    : m_bytes(immutableBytes)
{
    TU_ASSERT (m_bytes != nullptr);
    std::span<const tu_uint8> bytes(m_bytes->getData(), m_bytes->getSize());
    m_reader = std::make_shared<const internal::ManifestReader>(bytes);
}

zuri_packager::ZuriManifest::ZuriManifest(std::span<const tu_uint8> unownedBytes)
{
    m_reader = std::make_shared<const internal::ManifestReader>(unownedBytes);
}

zuri_packager::ZuriManifest::ZuriManifest(const ZuriManifest &other)
    : m_bytes(other.m_bytes),
      m_reader(other.m_reader)
{
}

bool
zuri_packager::ZuriManifest::isValid() const
{
    return m_reader && m_reader->isValid();
}

zuri_packager::ManifestVersion
zuri_packager::ZuriManifest::getABI() const
{
    if (!isValid())
        return ManifestVersion::Unknown;
    switch (m_reader->getABI()) {
        case zpk1::ManifestVersion::Version1:
            return ManifestVersion::Version1;
        case zpk1::ManifestVersion::Unknown:
        default:
            return ManifestVersion::Unknown;
    }
}

zuri_packager::ManifestWalker
zuri_packager::ZuriManifest::getManifest() const
{
    if (!isValid())
        return {};
    return ManifestWalker(m_reader);
}

std::shared_ptr<const zuri_packager::internal::ManifestReader>
zuri_packager::ZuriManifest::getReader() const
{
    return m_reader;
}

std::span<const tu_uint8>
zuri_packager::ZuriManifest::bytesView() const
{
    if (!isValid())
        return {};
    return m_reader->bytesView();
}

std::string
zuri_packager::ZuriManifest::dumpJson() const
{
    if (!isValid())
        return {};
    return m_reader->dumpJson();
}


bool
zuri_packager::ZuriManifest::verify(std::span<const tu_uint8> bytes)
{
    flatbuffers::Verifier verifier(bytes.data(), bytes.size());
    return zpk1::VerifyManifestBuffer(verifier);
}
