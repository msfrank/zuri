#ifndef ZURI_PACKAGER_PACKAGE_EXTRACTOR_H
#define ZURI_PACKAGER_PACKAGE_EXTRACTOR_H

#include <filesystem>

#include <tempo_utils/result.h>

#include "entry_walker.h"
#include "package_reader.h"
#include "package_result.h"

namespace zuri_packager {

    struct PackageExtractorOptions {
        /**
         *
         */
        std::filesystem::path distributionRoot = {};
        /**
         *
         */
        std::filesystem::path workingRoot = {};
    };

    class PackageExtractor {
    public:
        explicit PackageExtractor(const std::filesystem::path &packagePath, const PackageExtractorOptions &options = {});

        tempo_utils::Status configure();

        tempo_utils::Result<std::filesystem::path> extractPackage();

    private:
        std::filesystem::path m_packagePath;
        PackageExtractorOptions m_options;

        std::filesystem::path m_workdirPath;
        std::queue<EntryWalker> m_pendingDirectories;
        std::queue<EntryWalker> m_unresolvedLinks;

        tempo_utils::Status extractRoot(const EntryWalker &root, std::shared_ptr<PackageReader> reader);
        tempo_utils::Status extractChildren(
            const EntryWalker &parent,
            std::shared_ptr<PackageReader> reader);
        tempo_utils::Status extractFile(const EntryWalker &file, std::shared_ptr<PackageReader> reader);
        tempo_utils::Status linkEntry(const EntryWalker &link);
    };
}

#endif // ZURI_PACKAGER_PACKAGE_EXTRACTOR_H
