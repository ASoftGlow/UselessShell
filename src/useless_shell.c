#include "useless_shell.h"
#include "us_impl.h"
#include "ansi_codes.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>

#define _VERIFY(expression, action) if (!(expression)) {put(NO_COLOR ? "!ERROR! " :ANSI_COLOR_RED);action;if(!NO_COLOR)put(ANSI_COLOR_RESET);fflush(stdout);abort();}
#define VERIFY(expression, reason) _VERIFY(expression, puts(reason))
#if _DEBUG
#define VERIFY_CMD(expression, reason) _VERIFY(expression, printf("[CMD] %s: " reason "\n", cmd->name))
#define VERIFY_CMD_ARG(expression, reason) _VERIFY(expression, printf("[CMD] %s: [ARG] %s: " reason, cmd->name, arg->name))
#define _WARN(expression, action) if (expression) {put(NO_COLOR ? "!WARNING! " : ANSI_COLOR_YELLOW);action;if(!NO_COLOR)put(ANSI_COLOR_RESET);}
#define WARN(expression, reason) _WARN(expression, puts(reason));
#define WARN_CMD(expression, reason) _WARN(expression, printf("[CMD] %s: " reason "\n", cmd->name))
#define WARN_CMD_ARG(etype, reason) WARN(arg->type == USCommandArgType ## etype, "A " #etype " argument " reason)
#else
#define VERIFY_CMD(expression, reason)
#define VERIFY_CMD_ARG(expression, reason)
#define WARN(expression, reason)
#define WARN_CMD(expression, reason)
#define WARN_CMD_ARG(etype, reason)
#endif

#ifdef US_ENABLE_TESTING
static void _VERIFY_INPUT(US* us, const char* input, USProcessCmd expected, const char* purpose)
{
	strcpy(us->ibuff, input);
	put(ANSI_CLEAR_OUTPUT);
	const USProcessCmd actual = us_process_cmd(us, (int)strlen(input));
	_VERIFY(actual == expected, put(purpose); puts(USProcessCmd__Names[actual]));
}
#define VERIFY_INPUT(input, expected, purpose) \
	_VERIFY_INPUT(us, input, USProcessCmd ## expected, "[Testing] " purpose "\n  Input:\t" input "\n  Expected:\t" STR_VALUE(expected) "\n  Actual:\t");

static void us_run_tests(US* us)
{
	VERIFY_INPUT("_test a", Success, "Basic");
	VERIFY_INPUT("_test b one two", Success, "Basic arguments");
	VERIFY_INPUT("_test no_exist", ParseError, "Unknown cmd");
	VERIFY_INPUT("_test b", ParseError, "Missing arguments");
	VERIFY_INPUT("_test b one", ParseError, "Missing arguments");
	VERIFY_INPUT("_test a one", ParseError, "Extra argument");
	VERIFY_INPUT("_test c -a", Success, "Flag argument");
	VERIFY_INPUT("_test c -ab", Success, "Flag arguments");
	VERIFY_INPUT("_test c -ba", Success, "Flag arguments");
	VERIFY_INPUT("_test c one -ab", Success, "Flag arguments");
	VERIFY_INPUT("_test c -ab one", Success, "Flag arguments");
	VERIFY_INPUT("_test c -a one -b", Success, "Flag arguments");
	VERIFY_INPUT("_test -o a", ParseError, "Extra flag");
	VERIFY_INPUT("_test a -o", ParseError, "Extra flag");
	VERIFY_INPUT("_test -o a -o", ParseError, "Extra flag");
	VERIFY_INPUT("_test super", ExecError, "Super permission");
	VERIFY_INPUT("_test \"a\"", Success, "Quoted arguments");
	VERIFY_INPUT("_test b \"one\" two", Success, "Quoted arguments");
	VERIFY_INPUT("_test b \"one space\" two", Success, "Quoted arguments");
	VERIFY_INPUT("_test b \"one two", ParseError, "Incomplete quotes");
	VERIFY_INPUT("_test \"", ParseError, "Incomplete quotes");

	put(ANSI_CLEAR_OUTPUT);
}
#endif

static void us_verify_cmd(const USCommand* cmd)
{
	VERIFY_CMD(!(cmd->args_len && cmd->subcmds_len), "A command cannot have both arguments and sub commands");
	VERIFY_CMD(cmd->name[0], "A command must have a name");
	VERIFY_CMD(strislwr(cmd->name), "A command name must be lowercase");
	int j = -1;
	while (cmd->name[++j])
	{
		VERIFY_CMD(cmd->name[j] != ' ', "A command name must not contain spaces");
	}
	bool optional = false;
	for (short i = 0; i < cmd->args_len; i++)
	{
		const USCommandArg* arg = &cmd->args[i];
		VERIFY_CMD_ARG(arg->name[0], "An argument must have a name");
		VERIFY_CMD_ARG(strislwr(arg->name), "An argument name must be lowercase");
		int j = -1;
		while (arg->name[++j])
		{
			VERIFY_CMD_ARG(arg->name[j] != ' ', "An argument name must not contain spaces");
		}

		if (arg->auto_complete)
		{
			WARN_CMD_ARG(Boolean, "should not have an auto complete function");
			WARN_CMD_ARG(Secret, "should not have an auto complete function");
			WARN_CMD_ARG(Flag, "should not have an auto complete function");
		}

		VERIFY_CMD_ARG(arg->type != USCommandArgTypeUnset, "An argument's type must be set");
		if (arg->type == USCommandArgTypeFlag)
		{
			VERIFY_CMD_ARG(arg->name[1] == 0, "A Flag argument's name must be one character long");
			for (short k = 0; k < cmd->args_len; k++)
			{
				const USCommandArg* arg2 = &cmd->args[k];
				if (arg2->type == USCommandArgTypeFlag && arg != arg2)
				{
					VERIFY_CMD_ARG(arg->name[0] != arg2->name[0], "A Flag argument's name must be unique");
				}
			}
		}
		else
		{
			if (optional)
			{
				VERIFY_CMD(arg->optional, "All arguments preceding an optional argument must also be optional");
			}
			else if (arg->optional)
			{
				optional = true;
			}
		}
	}
	WARN_CMD(!cmd->subcmds_len && !cmd->impl, "No command implemention");
}

static void us_verify_cmds(const USCommand* cmds, short cmds_len)
{
	VERIFY(cmds_len > 0, "No commands are registered");
	for (int i = 0; i < cmds_len; i++)
	{
		const USCommand* cmd = &cmds[i];
		if (strcmp(cmd->name, "_test") == 0) continue;
		us_verify_cmd(cmd);
		for (int j = 0; j < cmd->subcmds_len; j++)
		{
			us_verify_cmd(&cmd->subcmds[j]);
		}
	}
}

_Ret_maybenull_ UselessShell* useless_shell_create(_In_reads_(cmds_len) const USCommand* cmds, int16_t cmds_len)
{
	US* us = (US*)malloc(sizeof(US));
	if (!us) return NULL;
	if (us_create(us, cmds, cmds_len))
	{
#ifdef US_VERIFY_COMMANDS
		us_verify_cmds(cmds, cmds_len);
#endif
#ifdef US_ENABLE_TESTING
		us_run_tests(us);
#endif
		return (UselessShell*)us;
	}
	free(us);
	return NULL;
}

void useless_shell_destroy(_Post_invalid_ _In_ UselessShell* us)
{
	us_destroy((US*)us);
	free(us);
}

bool useless_shell_start(_Inout_ UselessShell* us)
{
	return us_start((US*)us);
}

bool useless_shell_create_user(_Inout_ UselessShell* us, _In_z_ const char* username, _In_z_ const char* password, bool is_super, char icon)
{
	return us_create_user((US*)us, username, password, is_super, icon) != NULL;
}

bool useless_shell_delete_user(_Inout_ UselessShell* us, _In_z_ const char* username)
{
	User* user = us_get_user((US*)us, username);
	if (user)
	{
		return us_delete_user((US*)us, user);
	}
	return false;
}

bool useless_shell_login(_Inout_ UselessShell* us, _In_z_ const char* username)
{
	User* user = us_get_user((US*)us, username);
	if (user)
	{
		return us_login((US*)us, user, true);
	}
	return false;
}

bool useless_shell_logout(_Inout_ UselessShell* us)
{
	if (((US*)us)->current_user)
	{
		return us_logout((US*)us);
	}
	return false;
}