#ifndef ZURI_BUILD_IMPORT_RESOLVER_H
#define ZURI_BUILD_IMPORT_RESOLVER_H
#include <lyric_importer/importer_types.h>
#include <lyric_importer/shortcut_resolver.h>

#include "import_store.h"

class ImportResolver {
public:
    ImportResolver(std::shared_ptr<ImportStore> importStore);

private:
    std::shared_ptr<ImportStore> m_importStore;
    std::shared_ptr<lyric_importer::ShortcutResolver> m_shortcutResolver;
};

#endif // ZURI_BUILD_IMPORT_RESOLVER_H
