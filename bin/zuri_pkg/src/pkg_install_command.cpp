
#include <tempo_command/command_help.h>
#include <tempo_command/command_parser.h>
#include <tempo_config/abstract_converter.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/config_result.h>
#include <tempo_config/container_conversions.h>
#include <tempo_utils/result.h>
#include <zuri_distributor/dependency_selector.h>
#include <zuri_distributor/package_fetcher.h>
#include <zuri_packager/package_specifier.h>
#include <zuri_packager/package_types.h>
#include <zuri_pkg/pkg_install_command.h>
#include <zuri_pkg/pkg_result.h>

#include "zuri_pkg/install_solver.h"

struct PackageSpecifierOrIdOrUrl {
    enum class Type {
        Invalid,
        Id,
        Specifier,
        Url,
    };
    Type type;
    zuri_packager::PackageId packageId;
    zuri_packager::PackageSpecifier packageSpecifier;
    tempo_utils::Url packageUrl;
};

class PackageSpecifierOrIdOrUrlParser : public tempo_config::AbstractConverter<PackageSpecifierOrIdOrUrl> {
public:
    tempo_utils::Status convertValue(
        const tempo_config::ConfigNode &node,
        PackageSpecifierOrIdOrUrl &value) const override
    {
        if (node.getNodeType() == tempo_config::ConfigNodeType::kNil)
            return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kMissingValue,
                "missing required package id or specifier");
        if (node.getNodeType() != tempo_config::ConfigNodeType::kValue)
            return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kWrongType,
                "expected Value node but found {}", config_node_type_to_string(node.getNodeType()));

        auto string = node.toValue().getValue();

        auto authority = tempo_utils::UrlAuthority::fromString(string);
        if (authority.isValid()) {
            value.packageSpecifier = zuri_packager::PackageSpecifier::fromAuthority(authority);
            if (value.packageSpecifier.isValid()) {
                value.type = PackageSpecifierOrIdOrUrl::Type::Specifier;
                return {};
            }

            value.packageId = zuri_packager::PackageId::fromAuthority(authority);
            if (value.packageId.isValid()) {
                value.type = PackageSpecifierOrIdOrUrl::Type::Id;
                return {};
            }
        } else {
            value.packageUrl = tempo_utils::Url::fromString(string);
            if (value.packageUrl.isValid()) {
                value.type = PackageSpecifierOrIdOrUrl::Type::Url;
                return {};
            }
        }

        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kParseError,
            "'{}' is not a valid package id or specifier or url", string);
    }
};

tempo_utils::Status
zuri_pkg::pkg_install_command(
    const std::filesystem::path &distributionRoot,
    bool manageSystem,
    tempo_command::TokenVector &tokens)
{
    PackageSpecifierOrIdOrUrlParser packageSpecifierOrIdOrUrlParser;
    tempo_config::SeqTParser packagesParser(&packageSpecifierOrIdOrUrlParser, {});
    tempo_config::BooleanParser dryRunParser(false);

    std::vector<tempo_command::Default> defaults = {
        {"dryRun", "Display what would be installed but make no changes"},
        {"packages", "Packages to install", "PACKAGE"},
    };

    const std::vector<tempo_command::Grouping> groupings = {
        {"dryRun", {"--dry-run"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"help", {"-h", "--help"}, tempo_command::GroupingType::HELP_FLAG},
    };

    const std::vector<tempo_command::Mapping> optMappings = {
        {tempo_command::MappingType::TRUE_IF_INSTANCE, "dryRun"},
    };

    std::vector<tempo_command::Mapping> argMappings = {
        {tempo_command::MappingType::ONE_OR_MORE_INSTANCES, "packages"},
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
                tempo_command::display_help_and_exit({"zuri-pkg", "install"},
                    "Install a package",
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

    // load zuri config
    std::shared_ptr<zuri_tooling::ZuriConfig> zuriConfig;
    TU_ASSIGN_OR_RETURN (zuriConfig, zuri_tooling::ZuriConfig::forUser(
        {}, distributionRoot));

    // construct and configure the package manager
    auto packageManager = std::make_shared<zuri_tooling::PackageManager>(zuriConfig);
    TU_RETURN_IF_NOT_OK (packageManager->configure());

    bool dryRun;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(dryRun, dryRunParser,
        commandConfig, "dryRun"));

    InstallSolver installSolver(packageManager, manageSystem, dryRun);
    TU_RETURN_IF_NOT_OK (installSolver.configure());

    std::vector<PackageSpecifierOrIdOrUrl> packages;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(packages, packagesParser,
        commandConfig, "packages"));

    for (const auto &package : packages) {
        switch (package.type) {
            case PackageSpecifierOrIdOrUrl::Type::Id:
                TU_RETURN_IF_NOT_OK (installSolver.addPackage(package.packageId));
                break;
            case PackageSpecifierOrIdOrUrl::Type::Specifier:
                TU_RETURN_IF_NOT_OK (installSolver.addPackage(package.packageSpecifier));
                break;
            case PackageSpecifierOrIdOrUrl::Type::Url:
                TU_RETURN_IF_NOT_OK (installSolver.addPackage(package.packageUrl));
                break;
            default:
                break;
        }
    }

    return installSolver.installPackages();
}
