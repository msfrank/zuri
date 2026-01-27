
#include <stack>

#include <tempo_command/command.h>
#include <tempo_config/abstract_converter.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <tempo_utils/result.h>
#include <tempo_utils/url_path.h>
#include <zuri_distributor/package_fetcher.h>
#include <zuri_packager/package_reader.h>
#include <zuri_packager/package_specifier.h>
#include <zuri_packager/package_types.h>
#include <zuri_zpk/zpk_inspect_command.h>
#include <zuri_zpk/zpk_result.h>

tempo_utils::Status
zuri_zpk::zpk_inspect_command(
    std::shared_ptr<zuri_tooling::CoreConfig> coreConfig,
    tempo_command::TokenVector &tokens)
{
    tempo_config::PathParser zpkFileParser;
    tempo_config::BooleanParser displayAllParser(false);

    tempo_command::Command command(std::vector<std::string>{"zuri-zpk", "inspect"});

    command.addArgument("zpkFile", "FILE", tempo_command::MappingType::ONE_INSTANCE,
        "Package file to inspect");
    command.addFlag("displayAll", {"-a", "--all"}, tempo_command::MappingType::TRUE_IF_INSTANCE,
        "Display all information (overrides individual display flags)");
    command.addHelpOption("help", {"-h", "--help"},
        "Inspect the contents of a zpk file");

    TU_RETURN_IF_NOT_OK (command.parseCompletely(tokens));

    std::filesystem::path zpkFile;
    TU_RETURN_IF_NOT_OK (command.convert(zpkFile, zpkFileParser, "zpkFile"));

    std::shared_ptr<zuri_packager::PackageReader> reader;
    TU_ASSIGN_OR_RETURN (reader, zuri_packager::PackageReader::open(zpkFile));

    if (!reader->isValid())
        return ZpkStatus::forCondition(ZpkCondition::kZpkInvariant,
            "file '{}' is not a valid zpk file", zpkFile.string());

    // the zpk header

    TU_CONSOLE_OUT << "Header:";
    TU_CONSOLE_OUT << "  zpk version:   " << reader->getVersion();
    TU_CONSOLE_OUT << "  zpk flags:     " << reader->getFlags();
    TU_CONSOLE_OUT << "---";

    // the zpk manifest

    auto manifest = reader->getManifest();

    std::stack<zuri_packager::EntryWalker> stack;
    std::vector<std::pair<tempo_utils::UrlPath,zuri_packager::EntryWalker>> entries;
    stack.push(manifest.getRoot());
    while (!stack.empty()) {
        auto entry = stack.top();
        stack.pop();
        entries.emplace_back(entry.getPath(), entry);
        for (int i = 0; i < entry.numChildren(); i++) {
            stack.push(entry.getChild(i));
        }
    }
    std::sort(entries.begin(), entries.end(), [](auto &a, auto &b) -> bool {
        return a.first.toString() < b.first.toString();
    });

    TU_CONSOLE_OUT << "Manifest:";
    TU_CONSOLE_OUT << "  version:       " << (int) manifest.getABI();
    if (entries.empty()) {
        TU_CONSOLE_OUT << "  entries: none";
    } else {
        TU_CONSOLE_OUT << "  entries:";
        for (const auto &entry : entries) {
            TU_CONSOLE_OUT << "    " << entry.first.toString();
        }
    }
    TU_CONSOLE_OUT << "---";

    // the package metadata

    zuri_packager::PackageSpecifier packageSpecifier;
    TU_ASSIGN_OR_RETURN (packageSpecifier, reader->readPackageSpecifier());

    lyric_common::ModuleLocation programMain;
    TU_ASSIGN_OR_RETURN (programMain, reader->readProgramMain());

    zuri_packager::RequirementsMap requirementsMap;
    TU_ASSIGN_OR_RETURN (requirementsMap, reader->readRequirementsMap());
    std::vector<std::pair<zuri_packager::PackageId,zuri_packager::PackageVersion>> requirements;
    for (auto it = requirementsMap.requirementsBegin(); it != requirementsMap.requirementsEnd(); it++) {
        requirements.emplace_back(it->first, it->second);
    }
    std::sort(requirements.begin(), requirements.end(), [](auto &a, auto &b) -> bool {
        return a < b;
    });

    TU_CONSOLE_OUT << "Metadata:";
    TU_CONSOLE_OUT << "  specifier:     " << packageSpecifier.toString();
    TU_CONSOLE_OUT_IF(programMain.isValid()) << "  program main:  " << programMain.toString();
    if (requirements.empty()) {
        TU_CONSOLE_OUT << "  requirements:  none";
    } else {
        TU_CONSOLE_OUT << "  requirements:";
        for (const auto &requirement : requirements) {
            zuri_packager::PackageSpecifier specifier(requirement.first, requirement.second);
            TU_CONSOLE_OUT << "    " << specifier.toString();
        }
    }
    TU_CONSOLE_OUT << "---";

    // the package.config

    tempo_config::ConfigMap packageConfig;
    TU_ASSIGN_OR_RETURN (packageConfig, reader->readPackageConfig());

    TU_CONSOLE_OUT << "package.config:";
    std::string configJson;
    TU_RETURN_IF_NOT_OK (write_config_string(packageConfig, configJson));
    TU_CONSOLE_OUT << configJson;
    TU_CONSOLE_OUT << "---";

    return {};
}
