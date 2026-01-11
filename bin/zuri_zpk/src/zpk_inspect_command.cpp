
#include <stack>

#include <tempo_command/command_help.h>
#include <tempo_command/command_parser.h>
#include <tempo_config/abstract_converter.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/config_result.h>
#include <tempo_config/container_conversions.h>
#include <tempo_utils/result.h>
#include <tempo_utils/url_path.h>
#include <zuri_distributor/dependency_selector.h>
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

    std::vector<tempo_command::Default> defaults = {
        {"displayAll", "Display all information (overrides individual display flags)"},
        {"zpkFile", "Package file to inspect", "FILE"},
    };

    const std::vector<tempo_command::Grouping> groupings = {
        {"displayAll", {"-a", "--all"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"help", {"-h", "--help"}, tempo_command::GroupingType::HELP_FLAG},
    };

    const std::vector<tempo_command::Mapping> optMappings = {
        {tempo_command::MappingType::TRUE_IF_INSTANCE, "displayAll"},
    };

    std::vector<tempo_command::Mapping> argMappings = {
        {tempo_command::MappingType::ONE_INSTANCE, "zpkFile"},
    };

    tempo_command::OptionsHash options;
    tempo_command::ArgumentVector arguments;

    // parse global options and arguments
    auto status = tempo_command::parse_completely(tokens, groupings, options, arguments);
    if (status.notOk()) {
        tempo_command::CommandStatus commandStatus;
        if (!status.convertTo(commandStatus))
            return status;
        switch (commandStatus.getCondition()) {
            case tempo_command::CommandCondition::kHelpRequested:
                tempo_command::display_help_and_exit({"zuri-zpk", "inspect"},
                    "Inspect the contents of a zpk file",
                    {}, groupings, optMappings, argMappings, defaults);
            case tempo_command::CommandCondition::kVersionRequested:
                tempo_command::display_version_and_exit(PROJECT_VERSION);
            default:
                return status;
        }
    }

    tempo_command::CommandConfig commandConfig;

    // convert options to config
    TU_RETURN_IF_NOT_OK (tempo_command::convert_options(options, optMappings, commandConfig));

    // convert arguments to config
    TU_RETURN_IF_NOT_OK (tempo_command::convert_arguments(arguments, argMappings, commandConfig));

    // construct command map
    tempo_config::ConfigMap commandMap(commandConfig);

    std::filesystem::path zpkFile;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(zpkFile, zpkFileParser,
        commandConfig, "zpkFile"));

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
