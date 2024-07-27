#pragma once
#include "useless_shell.h"
#include "cmd_macros.h"

CMD_IMPL(clear);
CMD_IMPL(exit);
CMD_IMPL(help);
CMD_IMPL(history);
CMD_IMPL(history_clear);
CMD_IMPL(user_info);
CMD_IMPL(user_list);
CMD_IMPL(user_create);
CMD_IMPL(user_delete);
CMD_IMPL(login);
CMD_IMPL(logout);
CMD_IMPL(uninstall);
CMD_IMPL(time);

AC_IMPL(users);
AC_IMPL(cmds);