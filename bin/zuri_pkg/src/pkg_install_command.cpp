
#include <tempo_command/command.h>
#include <tempo_config/abstract_converter.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/config_result.h>
#include <tempo_config/container_conversions.h>
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
    std::shared_ptr<zuri_tooling::EnvironmentConfig> environmentConfig,
    std::shared_ptr<zuri_distributor::Runtime> runtime,
    tempo_command::TokenVector &tokens)
{
    PackageSpecifierOrIdOrUrlParser packageSpecifierOrIdOrUrlParser;
    tempo_config::SeqTParser packagesParser(&packageSpecifierOrIdOrUrlParser, {});
    tempo_config::BooleanParser dryRunParser(false);

    tempo_command::Command command(std::vector<std::string>{"zuri-pkg", "install"});

    command.addArgument("packages", "PACKAGE", tempo_command::MappingType::ONE_OR_MORE_INSTANCES,
        "Packages to install");
    command.addFlag("dryRun", {"--dry-run"}, tempo_command::MappingType::TRUE_IF_INSTANCE,
        "Display what would be installed but make no changes");
    command.addHelpOption("help", {"-h", "--help"},
        "Install a package");

    TU_RETURN_IF_NOT_OK (command.parseCompletely(tokens));

    bool dryRun;
    TU_RETURN_IF_NOT_OK (command.convert(dryRun, dryRunParser, "dryRun"));

    std::vector<PackageSpecifierOrIdOrUrl> packages;
    TU_RETURN_IF_NOT_OK (command.convert(packages, packagesParser, "packages"));

    InstallSolver installSolver(runtime, dryRun);
    TU_RETURN_IF_NOT_OK (installSolver.configure());

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
