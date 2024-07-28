#include "us_impl.h"

#include "cmd_macros.h"
#include "cmd_defs.h"
#include "cmd_impls.h"

const USCommand cmds[] =
{
	CMD(
		.name = "exit",
		.impl = CMD_IMPL(exit)
	),
	CMD(
		.name = "help",
		.impl = CMD_IMPL(help),
		CMD_ARGS(
			ARG(
				.name = "cmd",
				.type = USCommandArgTypeString,
				.optional = true,
				.auto_complete = AC_IMPL(cmds)
			),
			ARG(
				.name = "b",
				.type = USCommandArgTypeFlag
			)
		)
	),
	CMD(
		.name = "clear",
		.impl = CMD_IMPL(clear)
	),
	CMD(
		.name = "history",
		.description = "Display the session command history",
		.impl = CMD_IMPL(history),
		SUB_CMDS(
			CMD(
				.name = "clear",
				.impl = CMD_IMPL(history_clear)
			)
		)
	),
	CMD(
		.name = "login",
		.description = "Login to an existing account",
		.impl = CMD_IMPL(login),
		CMD_ARGS(
			ARG(
				.name = "username",
				.type = USCommandArgTypeString,
				.auto_complete = AC_IMPL(users)
			)
		)
	),
	CMD(
		.name = "logout",
		.description = "Logout of the currect account",
		.impl = CMD_IMPL(logout)
	),
	CMD(
		.name = "users",
		SUB_CMDS(
			CMD(
				.name = "info",
				.impl = CMD_IMPL(user_info),
				CMD_ARGS(
					ARG(
						.name = "username",
						.type = USCommandArgTypeString,
						.optional = true,
						.auto_complete = AC_IMPL(users)
					)
				)
			),
			CMD(
				.name = "list",
				.impl = CMD_IMPL(user_list)
			),
			CMD(
				.name = "create",
				.is_super = true,
				.impl = CMD_IMPL(user_create),
				CMD_ARGS(
					ARG(
						.name = "username",
						.type = USCommandArgTypeString
					),
					ARG(
						.name = "password",
						.type = USCommandArgTypeSecret
					),
					ARG(
						.name = "super",
						.type = USCommandArgTypeBoolean,
						.optional = true
					)
				)
			),
			CMD(
				.name = "delete",
				.is_super = true,
				.impl = CMD_IMPL(user_delete),
				CMD_ARGS(
					ARG(
						.name = "username",
						.type = USCommandArgTypeString,
						.auto_complete = AC_IMPL(users)
					)
				)
			)
		)
	),
	CMD(
		.name = "uninstall",
		.is_super = true,
		.impl = CMD_IMPL(uninstall)
	),
	CMD(
		.name = "time",
		.description = "Shows system time",
		.impl = CMD_IMPL(time),
		CMD_ARGS(
			ARG(
				.name = "a",
				.description = "Analog display",
				.type = USCommandArgTypeFlag
			),
			ARG(
				.name = "w",
				.description = "Watch mode - continuously update",
				.type = USCommandArgTypeFlag
			)
		)
	)
#ifdef US_ENABLE_TESTING
	,
	CMD(
		.name = "_test",
		SUB_CMDS(
			CMD(
				.name = "a"
			),
			CMD(
				.name = "b",
				CMD_ARGS(
					ARG(
						.name = "one",
						.type = USCommandArgTypeString
					),
					ARG(
						.name = "two",
						.type = USCommandArgTypeString
					)
				)
			),
			CMD(
				.name = "c",
				CMD_ARGS(
					ARG(
						.name = "one",
						.type = USCommandArgTypeString
					),
					ARG(
						.name = "a",
						.type = USCommandArgTypeFlag
					),
					ARG(
						.name = "b",
						.type = USCommandArgTypeFlag
					)
				)
			),
			CMD(
				.name = "super",
				.is_super = true
			)
		)
	)
#endif
};
const short cmds_len = countof(cmds);
