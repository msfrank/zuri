
#include <tempo_command/command_help.h>
#include <tempo_command/command_parser.h>
#include <tempo_config/abstract_converter.h>
#include <tempo_config/config_result.h>
#include <tempo_config/container_conversions.h>
#include <zuri_packager/package_specifier.h>
#include <zuri_packager/package_types.h>
#include <zuri_pkg/pkg_install.h>

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
zuri_pkg::pkg_install(tempo_command::TokenVector &tokens)
{
    PackageSpecifierOrIdOrUrlParser packageSpecifierOrIdOrUrlParser;
    tempo_config::SeqTParser packagesParser(&packageSpecifierOrIdOrUrlParser, {});

    std::vector<tempo_command::Default> defaults = {
        {"packages", {}, "Packages to install", "PACKAGE"},
    };

    const std::vector<tempo_command::Grouping> groupings = {
    };

    const std::vector<tempo_command::Mapping> optMappings = {
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
                display_help_and_exit({"zuri-pkg", "install"},
                    "Install a package",
                    {}, groupings, optMappings, argMappings, defaults);
            case tempo_command::CommandCondition::kVersionRequested:
                tempo_command::display_version_and_exit(PROJECT_VERSION);
            default:
                return status;
        }
    }

    // initialize the command config from defaults
    tempo_command::CommandConfig commandConfig = tempo_command::command_config_from_defaults(defaults);

    // convert options to config
    TU_RETURN_IF_NOT_OK (tempo_command::convert_options(options, optMappings, commandConfig));

    // convert arguments to config
    TU_RETURN_IF_NOT_OK (tempo_command::convert_arguments(arguments, argMappings, commandConfig));

    // construct command map
    tempo_config::ConfigMap commandMap(commandConfig);

    std::vector<PackageSpecifierOrIdOrUrl> packages;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(packages, packagesParser,
        commandConfig, "packages"));

    for (const auto &package : packages) {
        switch (package.type) {
            case PackageSpecifierOrIdOrUrl::Type::Id:
                TU_CONSOLE_OUT << "installing latest version of " << package.packageId.toString();
                break;
            case PackageSpecifierOrIdOrUrl::Type::Specifier:
                TU_CONSOLE_OUT << "installing " << package.packageSpecifier.toString();
                break;
            case PackageSpecifierOrIdOrUrl::Type::Url:
                TU_CONSOLE_OUT << "installing from url " << package.packageUrl.toString();
                break;
            default:
                break;
        }
    }

    return {};
}
