// #ifndef ZURI_PACKAGER_ENTRY_PATH_H
// #define ZURI_PACKAGER_ENTRY_PATH_H
//
// #include <string>
//
// #include <tempo_utils/log_message.h>
// #include <tempo_utils/url.h>
//
// namespace zuri_packager {
//
//     class EntryPath {
//     public:
//         EntryPath();
//         EntryPath(const EntryPath &other);
//
//         bool isValid() const;
//
//         bool isEmpty() const;
//
//         int numParts() const;
//         std::string getPart(int index) const;
//         std::string_view partView(int index) const;
//
//         EntryPath getInit() const;
//         EntryPath getTail() const;
//
//         std::string getFilename() const;
//         std::string_view filenameView() const;
//
//         std::string_view pathView() const;
//
//         EntryPath traverse(std::string_view part);
//
//         std::string toString() const;
//         tempo_utils::Url toUrl() const;
//
//         bool operator==(const EntryPath &other) const;
//         bool operator!=(const EntryPath &other) const;
//
//         static EntryPath fromString(std::string_view s);
//
//         template <typename H>
//         friend H AbslHashValue(H h, const EntryPath &path) {
//             return H::combine(std::move(h), path.m_path);
//         }
//
//     private:
//         tempo_utils::UrlPath m_path;
//
//         EntryPath(const tempo_utils::UrlPath &path);
//
//     public:
//         template<class... Args>
//         EntryPath traverse(std::string_view part, Args... args)
//         {
//             auto path = traverse(part);
//             return path.traverse(args...);
//         }
//     };
//
//     tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, const EntryPath &entryPath);
//
// }
//
// #endif // ZURI_PACKAGER_ENTRY_PATH_H