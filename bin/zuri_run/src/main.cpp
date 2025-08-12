/* SPDX-License-Identifier: AGPL-3.0-or-later */

#include <tempo_command/command_help.h>

#include <zuri_run/zuri_run.h>

int
main(int argc, const char *argv[]) {
    if (argc == 0 || argv == nullptr)
        return -1;

    auto status = zuri_run::zuri_run(argc, argv);
    if (!status.isOk())
        tempo_command::display_status_and_exit(status);
    return 0;
}