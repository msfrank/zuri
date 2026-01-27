
#include <tempo_command/command.h>
#include <zuri_pkg/pkg_cache_command.h>

tempo_utils::Status
zuri_pkg::pkg_cache_command(
    std::shared_ptr<zuri_tooling::EnvironmentConfig> environmentConfig,
    std::shared_ptr<zuri_distributor::Runtime> runtime,
    tempo_command::TokenVector &tokens)
{
    enum Subcommands {
        Info,
        List,
        NUM_SUBCOMMANDS,
    };
    std::vector<tempo_command::Subcommand> subcommands(NUM_SUBCOMMANDS);
    subcommands[Info] = {"info", "Display information about a specified package"};
    subcommands[List] = {"list", "List packages present in the cache"};

    tempo_command::Command command(std::vector<std::string>{"zuri-pkg", "cache"}, subcommands);

    command.addHelpOption("help", {"-h", "--help"},
        "Manage the package caches");

    int selected;
    TU_RETURN_IF_NOT_OK (command.parseUntilSubcommand(tokens, selected));

    return {};
}