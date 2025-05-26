/* SPDX-License-Identifier: AGPL-3.0-or-later */

#ifndef ZURI_BUILD_WORKSPACE_CONFIG_H
#define ZURI_BUILD_WORKSPACE_CONFIG_H

#include <tempo_config/program_config.h>
#include <tempo_config/workspace_config.h>

constexpr const char * const kEnvOverrideConfigName = "ZURI_BUILD_OVERRIDE_CONFIG";
constexpr const char * const kEnvOverrideVendorConfigName = "ZURI_BUILD_OVERRIDE_VENDOR_CONFIG";

tempo_utils::Result<std::shared_ptr<tempo_config::WorkspaceConfig>> find_workspace_config(
    const std::filesystem::path &searchPathStart,
    const std::filesystem::path &distributionRoot);

tempo_utils::Result<std::shared_ptr<tempo_config::WorkspaceConfig>> load_workspace_config(
    const std::filesystem::path &workspaceConfigFile,
    const std::filesystem::path &distributionRoot);

#endif // ZURI_BUILD_WORKSPACE_CONFIG_H
