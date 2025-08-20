/* SPDX-License-Identifier: AGPL-3.0-or-later */

#include <tempo_command/command_help.h>

#include <zuri_zpk/zuri_zpk.h>

int
main(int argc, const char *argv[]) {
    if (argc == 0 || argv == nullptr)
        return -1;

    auto status = zuri_zpk::zuri_zpk(argc, argv);
    if (!status.isOk())
        tempo_command::display_status_and_exit(status);
    return 0;
}