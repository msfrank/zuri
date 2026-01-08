
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
#include <zuri_env/env_create_command.h>
#include <zuri_env/env_result.h>

tempo_utils::Status
zuri_env::env_create_command(
    std::shared_ptr<zuri_tooling::ZuriConfig> zuriConfig,
    tempo_command::TokenVector &tokens)
{
    auto distribution = zuriConfig->getDistribution();
    auto home = zuriConfig->getHome();

    tempo_config::PathParser environmentRootParser(home.getEnvironmentsDirectory());
    tempo_config::PathParser componentRootParser;
    tempo_config::SeqTParser extraComponentDirsParser(&componentRootParser, {});
    tempo_config::BooleanParser dryRunParser(false);
    tempo_config::StringParser nameParser;

    std::vector<tempo_command::Default> defaults = {
        {"environmentRoot", "Environment root directory", "DIR"},
        {"extraComponentDirs", "Additional component directories", "DIR"},
        {"dryRun", "Display what would be installed but make no changes"},
        {"name", "The environment name", "NAME"},
    };

    const std::vector<tempo_command::Grouping> groupings = {
        {"environmentRoot", {"-E", "--environment-root"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"extraComponentDirs", {"--extra-distribution"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"dryRun", {"--dry-run"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"help", {"-h", "--help"}, tempo_command::GroupingType::HELP_FLAG},
    };

    const std::vector<tempo_command::Mapping> optMappings = {
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "environmentRoot"},
        {tempo_command::MappingType::ANY_INSTANCES, "extraComponentDirs"},
        {tempo_command::MappingType::TRUE_IF_INSTANCE, "dryRun"},
    };

    std::vector<tempo_command::Mapping> argMappings = {
        {tempo_command::MappingType::ONE_INSTANCE, "name"},
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
                tempo_command::display_help_and_exit({"zuri-env", "create"},
                    "Create an environment",
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

    std::filesystem::path environmentRoot;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(environmentRoot, environmentRootParser,
        commandConfig, "environmentRoot"));

    std::vector<std::filesystem::path> extraComponentDirs;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(extraComponentDirs, extraComponentDirsParser,
        commandConfig, "extraComponentDirs"));

    bool dryRun;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(dryRun, dryRunParser,
        commandConfig, "dryRun"));

    std::string name;
    TU_RETURN_IF_NOT_OK (tempo_command::parse_command_config(name, nameParser,
        commandConfig, "name"));

    auto envDirectory = environmentRoot / name;
    if (std::filesystem::exists(envDirectory))
        return EnvStatus::forCondition(EnvCondition::kEnvInvariant,
            "environment '{}' already exists in {}", name, environmentRoot.string());

    std::error_code ec;

    // create destination directory structure

    std::filesystem::create_directory(envDirectory, ec);
    if (ec)
        return EnvStatus::forCondition(EnvCondition::kEnvInvariant,
            "failed to create directory {}: {}", envDirectory.string(), ec.message());

    auto binDirectory = envDirectory / ZURI_RUNTIME_BIN_DIR;
    std::filesystem::create_directories(binDirectory, ec);
    if (ec)
        return EnvStatus::forCondition(EnvCondition::kEnvInvariant,
            "failed to create directory {}: {}", binDirectory.string(), ec.message());

    auto libDirectory = envDirectory / ZURI_RUNTIME_LIB_DIR;
    std::filesystem::create_directories(libDirectory, ec);
    if (ec)
        return EnvStatus::forCondition(EnvCondition::kEnvInvariant,
            "failed to create directory {}: {}", libDirectory.string(), ec.message());

    auto packagesDirectory = envDirectory / ZURI_RUNTIME_PACKAGES_DIR;
    std::filesystem::create_directories(packagesDirectory, ec);
    if (ec)
        return EnvStatus::forCondition(EnvCondition::kEnvInvariant,
            "failed to create directory {}: {}", packagesDirectory.string(), ec.message());

    auto environmentsDirectory = envDirectory / ZURI_RUNTIME_ENVIRONMENTS_DIR;
    std::filesystem::create_directories(environmentsDirectory, ec);
    if (ec)
        return EnvStatus::forCondition(EnvCondition::kEnvInvariant,
            "failed to create directory {}: {}", environmentsDirectory.string(), ec.message());

    auto configDirectory = envDirectory / ZURI_RUNTIME_CONFIG_DIR;
    std::filesystem::create_directories(configDirectory, ec);
    if (ec)
        return EnvStatus::forCondition(EnvCondition::kEnvInvariant,
            "failed to create directory {}: {}", configDirectory.string(), ec.message());

    // symlink binaries from distribution
    TU_RETURN_IF_NOT_OK (symlink_directory(distribution.getBinDirectory(), binDirectory));

    // symlink libraries from distribution
    TU_RETURN_IF_NOT_OK (symlink_directory(distribution.getLibDirectory(), libDirectory));

    // symlink entries from each component
    for (const auto &componentRoot : extraComponentDirs) {
        auto componentBin = componentRoot / ZURI_RUNTIME_BIN_DIR;
        TU_RETURN_IF_NOT_OK (symlink_directory(componentBin, binDirectory));
        auto componentLib = componentRoot / ZURI_RUNTIME_LIB_DIR;
        TU_RETURN_IF_NOT_OK (symlink_directory(componentLib, libDirectory));
    }

    return {};
}

tempo_utils::Status
zuri_env::symlink_directory(
    const std::filesystem::path &srcDirectory,
    const std::filesystem::path &dstDirectory)
{
    std::filesystem::directory_iterator srcIterator(srcDirectory);
    std::error_code ec;

    for (const auto &entry : srcIterator) {
        auto dstPath = dstDirectory / entry.path().filename();
        if (std::filesystem::exists(dstPath))
            return EnvStatus::forCondition(EnvCondition::kEnvInvariant,
                "failed to create symbolic link to {}: entry already exists", dstPath.string());

        if (entry.is_regular_file()) {
            std::filesystem::create_symlink(entry.path(), dstPath, ec);
        } else if (entry.is_symlink()) {
            std::filesystem::copy_symlink(entry.path(), dstPath, ec);
        } else {
            continue;
        }

        if (ec)
            return EnvStatus::forCondition(EnvCondition::kEnvInvariant,
                "failed to create symlink {}: {}", dstPath.string(), ec.message());
    }

    return {};
}