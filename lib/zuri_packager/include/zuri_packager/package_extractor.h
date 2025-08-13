#ifndef ZURI_PACKAGER_PACKAGE_EXTRACTOR_H
#define ZURI_PACKAGER_PACKAGE_EXTRACTOR_H

#include <filesystem>
#include <queue>

#include <tempo_utils/result.h>

#include "entry_walker.h"
#include "package_reader.h"
#include "package_specifier.h"

namespace zuri_packager {

    struct PackageExtractorOptions {
        /**
         *
         */
        std::filesystem::path destinationRoot = {};
        /**
         *
         */
        std::filesystem::path workingRoot = {};
    };

    class PackageExtractor {
    public:
        explicit PackageExtractor(std::shared_ptr<PackageReader> reader, const PackageExtractorOptions &options = {});

        tempo_utils::Status configure();

        tempo_utils::Result<std::filesystem::path> extractPackage();

    private:
        std::shared_ptr<PackageReader> m_reader;
        PackageExtractorOptions m_options;

        PackageSpecifier m_specifier;
        std::filesystem::path m_workdirPath;
        std::queue<EntryWalker> m_pendingDirectories;
        std::queue<EntryWalker> m_unresolvedLinks;

        tempo_utils::Status extractRoot(const EntryWalker &root);
        tempo_utils::Status extractChildren(const EntryWalker &parent);
        tempo_utils::Status extractFile(const EntryWalker &file);
        tempo_utils::Status linkEntry(const EntryWalker &link);
    };
}

#endif // ZURI_PACKAGER_PACKAGE_EXTRACTOR_H
