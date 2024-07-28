#pragma once
#include "useless_shell.h"
#include "cmd_macros.h"

CMD_IMPL_PROTO(clear);
CMD_IMPL_PROTO(exit);
CMD_IMPL_PROTO(help);
CMD_IMPL_PROTO(history);
CMD_IMPL_PROTO(history_clear);
CMD_IMPL_PROTO(user_info);
CMD_IMPL_PROTO(user_list);
CMD_IMPL_PROTO(user_create);
CMD_IMPL_PROTO(user_delete);
CMD_IMPL_PROTO(login);
CMD_IMPL_PROTO(logout);
CMD_IMPL_PROTO(uninstall);
CMD_IMPL_PROTO(time);

AC_IMPL_PROTO(users);
AC_IMPL_PROTO(cmds);