#include "us_impl.h"
#include "ansi_codes.h"
#include "base.h"
#include "util.h"
#include "Md5.h"

#include <string.h>
#include <time.h>

typedef struct
{
	DYNAMIC_ARRAY(cmds, const USCommand);
} ICommandProvider;

bool us_load_users(UselessShell* us);

static int User_serial_size = 0;

static void put_prefix(_In_ UselessShell* us)
{
	if (us->current_user)
	{
		if (!NO_COLOR)
			put(ANSI_COLOR_CYAN);
		put(us->current_user->username);
		if (!NO_COLOR)
			put(ANSI_COLOR_RESET);
		if (us->current_user->is_super)
		{
			put(":S> ");
		}
		else
		{
			put("> ");
		}
	}
	else
	{
		put("> ");
	}
}

bool us_create(_Out_ UselessShell* us, _In_reads_(cmds_len) const USCommand* cmds, int16_t cmds_len)
{
	us->cmds = cmds;
	us->cmds_len = cmds_len;

	ansi_setup();

	if (!User_serial_size)
	{
		User_serial_size = (int)((byte*)&us->users[0].__serial_end__ - (byte*)&us->users[0]);
	}
	us->current_user = NULL;
	for (int i = 0; i < countof(us->users); i++)
	{
		us->users[i].exists = false;
	}

	switch (get_cfg_path(us->cfg_path, ".useless_shell"))
	{
	case 0:
		us->mode = USModeCreated;
		us_load_users(us);
		break;

	case EEXIST:
		// new install
		us->mode = USModeNew;
		break;

	default:
		return false;
	}

	us->history.read = NULL;
	us->history.write = NULL;
	strcpy(us->history.path, us->cfg_path);
	strcat(us->history.path, "history");
	return true;
}

static int us_open_log(_In_ USLogFile* log)
{
	log->write = fopen(log->path, "a");
	log->read = fopen(log->path, "rb");
	if (!log->read || !log->write)
	{
		return -1;
	}
	setvbuf(log->write, NULL, _IONBF, 0);
	return 0;
}

static void us_close_log(_Inout_ USLogFile* log)
{
	if (log->read)
	{
		fclose(log->read);
		log->read = NULL;
	}
	if (log->write)
	{
		fclose(log->write);
		log->write = NULL;
	}
}

void us_destroy(_Inout_ UselessShell* us)
{
	us_close_log(&us->history);
	ansi_setdown();
	us->mode = USModeDestoryed;
}

_Success_(return) static bool split_cmd_s(_In_reads_(len) char* cmd, int len, _Out_ char* parts, int parts_size)
{
	int i = 0, h = 0, c = 0;
	bool str_escaped = false, back_escaped = false;
	while (i < len && cmd[i])
	{
		bool skip = false;
		switch (cmd[i])
		{
		case ' ':
			if (c)
			{
				if (!str_escaped)
				{
					parts[h++] = 0;
					c = 0;
					skip = true;
				}
			}
			else
			{
				skip = true;
			}
			break;

		case '\\':
			if (back_escaped)
			{
				break;
			}
			back_escaped = true;
			skip = true;
			break;

		case '\"':
			if (!back_escaped)
			{
				skip = true;
				str_escaped = !str_escaped;
			}
			break;

		default:
			break;
		}

		if (!skip)
		{
			back_escaped = false;
			parts[h++] = cmd[i];
			c++;
		}

		if (parts_size == h)
		{
			return false;
		}
		i++;
	}
	if (str_escaped || back_escaped || !h)
	{
		return false;
	}
	parts[h++] = 0;
	parts[h] = 0;
	return true;
}
#define split_cmd(cmd, len, parts) split_cmd_s(cmd, len, parts, countof(parts))

_Ret_maybenull_ static const ICommandProvider* _find_cmd(_In_ const ICommandProvider * provider, _Inout_ const char** arg_start)
{
	for (short i = 0; i < provider->cmds_len; i++)
	{
		const USCommand* cmd = &provider->cmds[i];
		if (strcmp(cmd->name, *arg_start) == 0)
		{
			*arg_start += strlen(*arg_start) + 1;
			if (*arg_start[0] == 0 || cmd->subcmds_len == 0)
			{
				return (const ICommandProvider*)cmd;
			}
			if (*arg_start[0])
			{
				const ICommandProvider* subcmd = _find_cmd((const ICommandProvider*)cmd, arg_start);
				if (subcmd) return subcmd;
			}
			break;
		}
	}
	return NULL;
}

_Ret_maybenull_ static const USCommand* find_cmd(_In_ const ICommandProvider * provider, _In_ const char* parts, _Out_ const char** arg_start)
{
	*arg_start = parts;
	const ICommandProvider* result = _find_cmd(provider, arg_start);
	return (const USCommand*)result;
}

_Ret_maybenull_ static inline const USCommand* us_find_cmd(_In_ UselessShell* us, _In_ const char* parts, _Out_ const char** arg_start)
{
	return find_cmd((const ICommandProvider*)us, parts, arg_start);
}

static void _tab_complete(_Inout_ USTabCompleteQuery * tc, _In_ const ICommandProvider * provider)
{
	for (short i = 0; i < provider->cmds_len; i++)
	{
		const USCommand* cmd = &provider->cmds[i];
		const size_t part_len = strlen(tc->part);
		if (strncmp(cmd->name, tc->part, part_len) == 0)
		{
			if (strlen(cmd->name) == part_len)
			{
				if (cmd->subcmds_len == 0)
				{
					if (cmd->args_len)
					{
						tc->part += part_len + 1;
						if (tc->part[0] || tc->is_whitespace_terminated)
						{
							// find last part
							int a = 0;
							while ((tc->part + strlen(tc->part) + 1)[0])
							{
								// flag
								if (tc->part[0] != '-' || cmd->args[a].type == USCommandArgTypeFlag)
								{
									a++;
								}
								tc->part += strlen(tc->part) + 1;
							}

							if (a < cmd->args_len)
							{
								// flag
								if (tc->part[0] == '-')
								{
									// ignore if last part is a flag
									if (tc->is_whitespace_terminated)
									{
										tc->part += strlen(tc->part) + 1;
									}
									else
									{
										tc->count = 0;
										tc->part++;
										short fi;
										for (short f = 0; f < cmd->args_len; f++)
										{
											USCommandArg* arg2 = &cmd->args[f];
											if (arg2->type == USCommandArgTypeFlag)
											{
												fi = 0;
												while (tc->part[fi])
												{
													if (tc->part[fi] == arg2->name[0])
													{
														goto found_flag;
													}
													fi++;
												}
												if (tc->count == countof(tc->results))
												{
													tc->count++;
													return;
												}
												tc->results[tc->count++] = arg2->name;
											found_flag:;
											}
										}
										tc->part++;
										return;
									}
								}
								const USCommandArg* arg = &cmd->args[a];
								if (arg->auto_complete)
								{
									arg->auto_complete(tc);
									if (tc->count == 1 && strcmp(tc->part, tc->results[0]) == 0)
									{
										tc->count = 0;
									}
									return;
								}
							}
						}
					}
					continue;
				}

				tc->part += part_len + 1;
				if (tc->part[0])
				{
					_tab_complete(tc, (const ICommandProvider*)cmd);
				}
				else if (tc->is_whitespace_terminated)
				{
					for (int j = 0; j < cmd->subcmds_len; j++)
					{
						if (tc->count == countof(tc->results))
						{
							tc->count++;
							return;
						}
						tc->results[tc->count++] = cmd->subcmds[j].name;
					}
				}
				break;
			}

			if (!(tc->part + part_len + 1)[0] && !tc->is_whitespace_terminated)
			{
				if (tc->count == countof(tc->results))
				{
					tc->count++;
					return;
				}
				tc->results[tc->count++] = cmd->name;
			}
		}
	}
}

static void tab_complete(_Inout_ USTabCompleteQuery * tc, _In_ const ICommandProvider * provider, _In_ char* search, int len)
{
	memset(tc->parts, 0, sizeof(tc->parts));
	tc->part = tc->parts;
	tc->is_whitespace_terminated = search[len - 1] == ' ';

	if (!split_cmd(search, len, tc->parts))
	{
		tc->count = 0;
		return;
	}
	_tab_complete(tc, provider);
	tc->offset = (byte)strlen(tc->part);
}

static inline void us_tab_complete(_Inout_ USTabCompleteQuery * tc, _In_ char* search, int len)
{
	tab_complete(tc, (const ICommandProvider*)tc->us, search, len);
}

typedef struct
{
	char* part;
	char result[96];
	bool is_whitespace_terminated;
} DetailsQuery;

static void _get_details(_Inout_ DetailsQuery * dq, _In_ const ICommandProvider * provider)
{
	for (short i = 0; i < provider->cmds_len; i++)
	{
		const USCommand* cmd = &provider->cmds[i];
		if (strcmp(cmd->name, dq->part) == 0)
		{
			if (cmd->subcmds_len == 0)
			{
				if (cmd->args_len && !(!dq->part[strlen(dq->part) + 1] && !dq->is_whitespace_terminated))
				{
					const USCommandArg* arg = NULL;
					dq->part += strlen(dq->part) + 1;
					if (dq->part[0])
					{
						int a = 0;
						while ((dq->part + strlen(dq->part) + 1)[0])
						{
							dq->part += strlen(dq->part) + 1;
							if (dq->part[0] != '-' || cmd->args[a].type == USCommandArgTypeFlag)
							{
								a++;
							}
						}

						if (a < cmd->args_len && dq->is_whitespace_terminated)
						{
							a++;
						}
						if (a >= cmd->args_len) return;
						// flag
						if (dq->part[0] == '-')
						{
							// ignore if last part is a flag
							if (dq->is_whitespace_terminated) return;

							for (short f = 0; f < cmd->args_len; f++)
							{
								USCommandArg* arg2 = &cmd->args[f];
								if (arg2->type == USCommandArgTypeFlag && arg2->name[0] == dq->part[1])
								{
									arg = arg2;
									break;
								}
							}
							if (arg)
							{
								dq->result[0] = '-';
								strcpy(dq->result + 1, arg->name);
								if (arg->description[0])
								{
									strcatc(dq->result, '\n');
									strcat(dq->result, arg->description);
								}
							}
							return;
						}
						else
						{
							arg = &cmd->args[a];
						}
					}
					else
					{
						for (short f = 0; f < cmd->args_len; f++)
						{
							USCommandArg* arg2 = &cmd->args[f];
							if (arg2->type == USCommandArgTypeFlag && arg2->name[0] == dq->part[1])
							{
								arg = arg2;
								break;
							}
						}
						if (!arg) return;
					}

					dq->result[0] = arg->optional ? '(' : '<';
					strcpy(dq->result + 1, arg->name);
					strcatc(dq->result, arg->optional ? ')' : '>');
					if (arg->description[0])
					{
						strcatc(dq->result, '\n');
						strcat(dq->result, arg->description);
					}
					break;
				}
				strcpy(dq->result, cmd->name);
				if (cmd->description[0])
				{
					strcatc(dq->result, '\n');
					strcat(dq->result, cmd->description);
				}
				continue;
			}

			dq->part += strlen(dq->part) + 1;
			if (dq->part[0])
			{
				_get_details(dq, (const ICommandProvider*)cmd);
			}
			else
			{
				strcpy(dq->result, cmd->name);
				strcatc(dq->result, '\n');
				strcat(dq->result, cmd->description);
			}
			break;
		}
	}
}

static bool get_details(_Inout_ DetailsQuery * dq, _In_ const ICommandProvider * provider, _In_z_ char* search, int len)
{
	char parts[32 * 16];
	dq->result[0] = 0;
	dq->is_whitespace_terminated = search[len - 1] == ' ';

	if (!split_cmd(search, len, parts))
	{
		return false;
	}
	dq->part = parts;
	_get_details(dq, provider);
	return dq->result[0];
}

static inline bool us_get_details(UselessShell* us, DetailsQuery * dq, char* search, int len)
{
	return get_details(dq, (const ICommandProvider*)us, search, len);
}

_Check_return_ USReturn us_get_secret(_Out_writes_(max) char* buffer, byte min, byte max)
{
	int i = 0;
	while (1)
	{
		USChar c = us_getchar();
		switch (c)
		{
		case USCharExit:
			put("\r" ANSI_CLEAR_PROCEEDING_LINE);
			return USReturnExit;

		case '\n':
		case '\r':
		case (char)EOF:
			if (i < min)
			{
				NOT_ALLOWED();
				break;
			}
			buffer[i] = 0;
			put("\r" ANSI_CLEAR_PROCEEDING_LINE);
			return USReturnNormal;

		case USCharEscape:
			put("\r" ANSI_CLEAR_PROCEEDING_LINE);
			return USReturnCancel;

		case '\b':
			if (i)
			{
				i--;
				put("\b \b");
				fflush(stdout);
			}
			else
			{
				NOT_ALLOWED();
			}
			break;

		case USCharBackWord:
			while (i)
			{
				i--;
				put("\b \b");
			}
			fflush(stdout);
			break;

		case '\t':
			NOT_ALLOWED();
			break;

		case USCharUndo:
			break;
		case USCharRedo:
			break;

		default:
			if (c < 256 && i < max - 1)
			{
				buffer[i++] = (char)c;
				putchar('*');
				fflush(stdout);
			}
			else
			{
				NOT_ALLOWED();
			}
			break;
		}
	}
}

_Ret_maybenull_ User* us_get_user(_In_ UselessShell* us, _In_z_ const char* username)
{
	for (int i = 0; i < countof(us->users); i++)
	{
		if (us->users[i].exists && strcmp(username, us->users[i].username) == 0)
		{
			return &us->users[i];
		}
	}
	return NULL;
}

static int us_save_user(_In_ UselessShell* us, _In_ User * user)
{
	char path[_MAX_PATH];
	strcpy(path, us->cfg_path);
	strcat(path, "users\\");
	create_directory(path);
	strcat(path, user->username);
	create_directory(path);
	strcat(path, "\\info");

	FILE* f = fopen(path, "wb");
	if (f)
	{
		size_t written = fwrite(user, User_serial_size, 1, f);
		fclose(f);
		if (written < 1) return -1;
		return 0;
	}
	return -1;
}

_Ret_maybenull_ User* us_create_user(_Inout_ UselessShell* us, _In_z_ const char* username, _In_z_ const char* password, bool is_super, char icon)
{
	MD5_CTX md5;
	MD5Init(&md5);
	MD5Update(&md5, password, strlen(password));
	MD5Final(&md5);
	return us_create_user_h(us, username, md5.digest, is_super, icon);
}

_Ret_maybenull_ User* us_create_user_h(_Inout_ UselessShell* us, _In_z_ const char* username, _In_ const char* password_hash, bool is_super, char icon)
{
	if (us_get_user(us, username))
	{
		puts(STYLE_ERROR("User already exists"));
		return false;
	}
	for (int i = 0; i < countof(us->users); i++)
	{
		User* user = &us->users[i];
		if (!user->exists)
		{
			user->exists = true;
			user->creation = time(NULL);
			user->last_login = 0;
			user->is_super = is_super;
			strcpy_s(user->username, sizeof(user->username), username);
			memcpy(user->password, password_hash, sizeof(user->password));
			user->icon = icon;

			us_save_user(us, user);
			return user;
		}
	}
	return NULL;
}

bool us_delete_user(_Inout_ UselessShell* us, _In_ User * user)
{
	if (strcmp(us->current_user->username, "admin") == 0)
	{
		puts(STYLE_ERROR("Cannot delete admin"));
		return false;
	}

	if (us->current_user == user)
	{
		us_logout(us);
	}

	char path[_MAX_PATH];
	strcpy(path, us->cfg_path);
	strcat(path, "users\\");
	create_directory(path);
	strcat(path, user->username);
	if (delete_directory(path) == 0)
		return true;

	user->exists = false;
	return false;
}

bool us_load_users(UselessShell* us)
{
	char path[_MAX_PATH];
	char contents[8][16];

	strcpy(path, us->cfg_path);
	strcat(path, "users");
	create_directory(path);

	if (!get_directory_contents(path, contents, countof(contents), true))
	{
		strcat(path, "\\");
		char* base = path + strlen(path);
		int i = 0;
		while (contents[i][0] && i < countof(contents))
		{
			strcpy(base, contents[i]);
			strcat(base, "\\info");
			FILE* f = fopen(path, "rb+");
			if (!f)
			{
				put(STYLE_ERROR_START "Failed to load user \"");
				put(contents[i]);
				puts("\"." STYLE_ERROR_END);
			}
			else
			{
				if (fread(&us->users[i], User_serial_size, 1, f) < 1)
				{
					put(STYLE_ERROR_START "Failed to load user \"");
					put(contents[i]);
					puts("\"" STYLE_ERROR_END);
				}
				fclose(f);
			}
			i++;
		}
		return true;
	}
	return false;
}

/**
 * @param line_start - pointer to start of line within {buffer}
 * @param from_pos - relative to the log's end
 * @returns lenth of line
 */
_Success_(return)
static int us_log_read_last(_Inout_ USLogFile * log, _Out_writes_(USInputBufferSize) char* buffer, _Out_ char** line_start, _Inout_ long* from_pos)
{
	if (fseek(log->read, 0, SEEK_END)) return 0;
	long fpos = ftell(log->read);
	if (*from_pos == fpos) return 0;
	long offset = *from_pos + USInputBufferSize;
	long read;
	if (fpos - offset < 0)
	{
		if (fseek(log->read, 0, SEEK_SET)) return 0;
		read = (long)fread(buffer, 1, USInputBufferSize + fpos - offset, log->read);
	}
	else
	{
		if (fseek(log->read, -offset, SEEK_END)) return 0;
		read = (long)fread(buffer, 1, USInputBufferSize, log->read);
	}
	if (!read) return 0;
	long i = read -= 2;
	if (buffer[i] == '\r')
	{
		(*from_pos)++;
		read--;
		i--;
	}
	while (--i >= 0 && buffer[i] != '\n')
	{
	}
	int len = read - i;
	*from_pos += len + 1;
	*line_start = buffer + i + 1;
	*((*line_start) + len) = 0;
	strcpy(log->last_line, *line_start);
	return len;
}

/**
 * @param start - pointer to start of line within {buffer}
 * @param from_pos - relative to the log's end
 * @returns lenth of line
 */
_Success_(return)
static int us_log_read_next(_Inout_ USLogFile * log, _Out_writes_(USInputBufferSize) char* buffer, _Out_ char** line_start, _Inout_ long* from_pos)
{
	if (*from_pos <= 0)
	{
		*from_pos = 0;
		return 0;
	}
	if (fseek(log->read, -*from_pos, SEEK_END)) return 0;
	long read = (long)fread(buffer, 1, USInputBufferSize, log->read);
	if (!read) return 0;
	--*from_pos;
	int offset = 1;
	long i = 0;
	while (true)
	{
		i++;
		if (i == read)
		{
			return 0;
		}
		if (buffer[i - 1] == '\r')
		{
			--*from_pos;
			offset++;
			continue;
		}
		if (buffer[i] == '\n' || (buffer[i] == '\r' && (buffer[i + 1] == '\n' || i + 1 == read)))
		{
			*line_start = buffer + offset;
			buffer[i] = 0;
			strcpy(log->last_line, *line_start);
			return i - offset;
		}
	}
}

static int us_open_history(UselessShell* us)
{
	if (us_open_log(&us->history))
	{
		puts("Failed to open cfg!");
		return -1;
	}

	char* line_start;
	long start_pos = 0;
	us_log_read_last(&us->history, us->lbuff, &line_start, &start_pos);
	return 0;
}

bool us_login(_Inout_ UselessShell* us, _In_ User * user, bool quiet)
{
	if (us->current_user)
	{
		if (us->current_user == user)
		{
			if (!quiet)
			{
				puts("You are already logged-in.");
			}
			return true;
		}
		if (!us_logout(us)) return false;
	}

	us->current_user = user;

	strcpy(us->history.path, us->cfg_path);
	strcat(us->history.path, "users\\");
	strcat(us->history.path, user->username);
	strcat(us->history.path, "\\history");
	if (us_open_history(us)) return false;

	if (!quiet)
	{
		put("Logged in as ");
		if (!NO_COLOR)
			put(ANSI_COLOR_CYAN);
		put(user->username);
		if (!NO_COLOR)
			put(ANSI_COLOR_RESET);
		if (user->last_login)
		{
			put(". Last login was on ");
			char* t = asctime(localtime(&user->last_login));
			if (!t) return false;
			t[strlen(t) - 1] = 0;
			put(t);
		}
		put(".\n");
	}

	user->last_login = time(NULL);
	return true;
}

bool us_logout(_Inout_ UselessShell* us)
{
	if (us_save_user(us, us->current_user)) return false;
	us_close_log(&us->history);
	us->current_user = NULL;
	strcpy(us->history.path, us->cfg_path);
	strcat(us->history.path, "history");
	if (us_open_history(us)) return false;
	return true;
}

_Check_return_ USProcessCmd us_process_cmd(_Inout_ UselessShell* us, int len)
{
	USProcessCmd ret = USProcessCmdParseError;
	char parts[USInputBufferSize];
	if (split_cmd(us->ibuff, len, parts))
	{
		const char* r_arg;
		const USCommand* cmd = us_find_cmd(us, parts, &r_arg);
		if (cmd)
		{
			USCommandArgValue* f_args = NULL;
			int a;

			if (cmd->is_super && !(us->current_user && us->current_user->is_super))
			{
				puts(STYLE_ERROR("Insufficient permission"));
				return USProcessCmdExecError;
			}

			if (cmd->args_len)
			{
				f_args = (USCommandArgValue*)malloc(cmd->args_len * sizeof(USCommandArgValue));
				assert(f_args);
				memset(f_args, 0, cmd->args_len * sizeof(USCommandArgValue));
				for (a = 0; a < cmd->args_len; a++)
				{
					USCommandArg* d_arg = &cmd->args[a];
					USCommandArgValue* arg = &f_args[a];

					if (r_arg[0])
					{
						if (r_arg[0] == '-')
						{
							short f = 0;
							while (r_arg[++f])
							{
								for (short i = 0; i < cmd->args_len; i++)
								{
									const USCommandArg* arg2 = &cmd->args[i];
									if (arg2->type == USCommandArgTypeFlag && r_arg[f] == arg2->name[0])
									{
										f_args[i].exists = true;
										goto found_flag;
									}
								}
								put(STYLE_ERROR_START "Unknown flag " STYLE_ARG);
								put(r_arg + 1);
								puts(STYLE_ERROR_END);
								goto arg_error;
							found_flag:;
							}
						}
						else
						{
							switch (d_arg->type)
							{
							case USCommandArgTypeString:
								int size = (int)strlen(r_arg) + 1;
								char* str = (char*)malloc(size);
								assert(str);
								memcpy(str, r_arg, size);
								arg->string = str;
								break;

							case USCommandArgTypeSecret:
							{
								MD5_CTX md5;
								MD5Init(&md5);
								MD5Update(&md5, r_arg, strlen(r_arg));
								MD5Final(&md5);
								char* str = (char*)malloc(sizeof(md5.digest));
								assert(str);
								memcpy(str, md5.digest, sizeof(md5.digest));
								arg->string = str;
							}
							break;

							case USCommandArgTypeNumber:
								if (r_arg[0] == '0' && r_arg[1] == 0)
								{
									arg->exists = SIZE_MAX;
									arg->number = 0;
									break;
								}
								char* end;
								errno = 0;
								long num = strtol(r_arg, &end, 10);
								if (!num || errno)
								{
									put(STYLE_ERROR_START "Invalid argument " STYLE_ARG);
									put(d_arg->name);
									puts(ANSI_COLOR_RED "; expected number" STYLE_ERROR_END);
									goto arg_error;
								}
								arg->number = num;
								break;

							case USCommandArgTypeBoolean:
								arg->exists = SIZE_MAX;
								if (
									strcmp(r_arg, "0")
									|| strcmp(r_arg, "f")
									|| strcmp(r_arg, "F")
									|| strcmp(r_arg, "false")
									|| strcmp(r_arg, "False")
									|| strcmp(r_arg, "FALSE")
									|| strcmp(r_arg, "OFF")
									|| strcmp(r_arg, "off")
									)
								{
									arg->boolean = true;
									break;
								}
								if (
									strcmp(r_arg, "1")
									|| strcmp(r_arg, "t")
									|| strcmp(r_arg, "T")
									|| strcmp(r_arg, "true")
									|| strcmp(r_arg, "True")
									|| strcmp(r_arg, "TRUE")
									|| strcmp(r_arg, "ON")
									|| strcmp(r_arg, "on")
									)
								{
									arg->boolean = true;
									break;
								}
								put(STYLE_ERROR_START "Invalid argument " STYLE_ARG);
								put(d_arg->name);
								puts(ANSI_COLOR_RED "; expected boolean" STYLE_ERROR_END);
								goto arg_error;

							case USCommandArgTypeFlag:
								break;
							}
						}

						r_arg += strlen(r_arg) + 1;
					}
					else if (!d_arg->optional && d_arg->type != USCommandArgTypeFlag)
					{
						put(STYLE_ERROR_START "Missing required argument " STYLE_ARG);
						put(d_arg->name);
						puts(STYLE_ERROR_END);
						goto arg_error;
					}
				}
			}
			if (r_arg[0])
			{
				put(STYLE_ERROR_START "Unknown argument " STYLE_ARG);
				put(r_arg);
				puts(STYLE_ERROR_END);
				goto arg_error;
			}

			ret = USProcessCmdSuccess;
			if (cmd->impl)
			{
				switch (cmd->impl((void*)us, f_args))
				{
				case USCommandReturnExit:
					ret = USProcessCmdExit;
					break;
				case USCommandReturnError:
					ret = USProcessCmdExecError;
					break;
				case USCommandReturnNormal:
					break;
				}
			}

			a = cmd->args_len;
		arg_error:
			if (cmd->args_len)
			{
				while (a--)
				{
					const USCommandArgValue* val = &f_args[a];
					const USCommandArg* arg = &cmd->args[a];
					if (val->exists)
					{
						switch (arg->type)
						{
						case USCommandArgTypeNumber:
							break;

						case USCommandArgTypeString:
						case USCommandArgTypeSecret:
							free(val->string);
							break;

						case USCommandArgTypeBoolean:
						case USCommandArgTypeFlag:
						default:
							break;
						}
					}
				}
				free(f_args);
			}
		}
		else
		{
			puts(STYLE_ERROR("Unknown command"));
			NOT_ALLOWED();
		}
	}
	else
	{
		puts(STYLE_ERROR("Invalid arguments"));
		NOT_ALLOWED();
	}
	return ret;
}

bool us_start(_Inout_ UselessShell* us)
{
	if (us->mode == USModeNew)
	{
		put("Enter a password for admin: ");
		fflush(stdout);
		char password[20] = { 0 };
		switch (us_get_secret(password, 3, sizeof(password)))
		{
		case USReturnCancel:
		case USReturnExit:
			delete_directory(us->cfg_path);
			return false;
		case USReturnNormal:
			break;
		}
		User* admin = us_create_user(us, "admin", password, true, '@');
		if (!admin) return false;
		if (!us_login(us, admin, true)) return false;
		us->mode = USModeCreated;
	}

	us_open_history(us);

	put("\nWelcome, ");
	if (!NO_COLOR)
		put(ANSI_COLOR_CYAN);
	put(us->current_user ? us->current_user->username : "Guest");
	if (!NO_COLOR)
		put(ANSI_COLOR_RESET);
	put(".\n\nToday is ");
	time_t t = time(NULL);
	const struct tm* lt = localtime(&t);
	char* const ti = asctime(lt);
	if (!ti) return false;
	ti[strlen(ti) - 1] = 0;
	put(ti);
	putchar('.');
	if (lt->tm_hour < 5)
	{
		put(" You should sleep.");
	}
	putchar('\n');

	while (true)
	{
		put_prefix(us);
		fflush(stdout);
		USChar c;
		int pos = 0;
		int len = 0;
		long history_pos = 0, history_line_len = 0;
		bool displaying_details = false;
		do
		{
			c = us_getchar();
			if (displaying_details)
			{
				displaying_details = false;
				put("\n" ANSI_CLEAR_PROCEEDING ANSI_CURSOR_RESTORE);
				if (c == USCharEscape)
				{
					fflush(stdout);
					continue;
				}
			}
			switch (c)
			{
			case USCharExit:
				if (us->current_user)
				{
					us_logout(us);
				}
				return true;

			case '\r':
			case '\n':
				c = 0;
				putchar('\n');
				break;

			case USCharHome:
				pos = 0;
				putchar('\r');
				put_prefix(us);
				break;

			case USCharEnd:
				if (pos < len)
				{
					printf("\33[%iC", len - pos);
					pos = len;
				}
				break;

			case '\b':
				if (pos)
				{
					if (pos != len)
					{
						put("\b" ANSI_CURSOR_SAVE);
						fwrite(us->ibuff + pos, 1, (size_t)len - pos, stdout);
						put(" " ANSI_CURSOR_RESTORE);
						memmove(us->ibuff + pos - 1, us->ibuff + pos, (size_t)len - pos);
					}
					else
					{
						put("\b \b");
					}
					len--;
					pos--;
				}
				else
				{
					NOT_ALLOWED();
				}
				break;

			case USCharBackWord:
				if (pos)
				{
					int del_pos = pos;
					while (--del_pos)
					{
						if (iswordend(us->ibuff[del_pos - 1])) break;
					}
					printf("\33[%iD", pos - del_pos);

					if (len == pos)
					{
						put(ANSI_CLEAR_PROCEEDING_LINE);
					}
					else
					{
						memmove(us->ibuff + del_pos, us->ibuff + pos, (size_t)len - pos);
						put(ANSI_CURSOR_SAVE);
						fwrite(us->ibuff + pos, 1, (size_t)len - pos, stdout);
						put(ANSI_CLEAR_PROCEEDING_LINE ANSI_CURSOR_RESTORE);
					}
					len -= pos - del_pos;
					pos = del_pos;
				}
				else
				{
					NOT_ALLOWED();
				}
				break;

			case USCharDelete:
				if (pos < len)
				{
					if (pos == len - 1)
					{
						put(" \b");
					}
					else
					{
						put(ANSI_CURSOR_SAVE);
						fwrite(us->ibuff + pos + 1, 1, (size_t)len - pos - 1, stdout);
						put(" " ANSI_CURSOR_RESTORE);
						memmove(us->ibuff + pos, us->ibuff + pos + 1, (size_t)len - pos - 1);
					}
					len--;
				}
				else
				{
					NOT_ALLOWED();
				}
				break;

			case USCharDeleteWord:
				if (pos < len)
				{
					int del_pos = pos;
					while (del_pos < len && !iswordend(us->ibuff[del_pos++])) {}

					memmove(us->ibuff + pos, us->ibuff + del_pos, (size_t)len - del_pos);
					put(ANSI_CURSOR_SAVE);
					fwrite(us->ibuff + pos, 1, (size_t)len - del_pos, stdout);
					put(ANSI_CLEAR_PROCEEDING_LINE ANSI_CURSOR_RESTORE);

					len -= del_pos - pos;
				}
				else
				{
					NOT_ALLOWED();
				}
				break;

			case USCharRight:
				if (pos < len)
				{
					pos++;
					put(ANSI_CURSOR_RIGHT(1));
				}
				else
				{
					NOT_ALLOWED();
				}
				break;

			case USCharRightWord:
				if (pos < len)
				{
					int del_pos = pos;
					while (del_pos < len && !iswordend(us->ibuff[del_pos++])) {}

					printf("\33[%iC", del_pos - pos);
					pos = del_pos;
				}
				else
				{
					NOT_ALLOWED();
				}
				break;

			case USCharLeft:
				if (pos)
				{
					pos--;
					put(ANSI_CURSOR_LEFT(1));
				}
				else
				{
					NOT_ALLOWED();
				}
				break;

			case USCharLeftWord:
				if (pos)
				{
					int del_pos = pos;
					while (del_pos)
					{
						del_pos--;

						if (iswordend(us->ibuff[del_pos - 1])) break;
					}
					printf("\33[%iD", pos - del_pos);
					pos = del_pos;
				}
				else
				{
					NOT_ALLOWED();
				}
				break;

			case '\t':
			{
				if (!len)
				{
					NOT_ALLOWED();
					break;
				}
				USTabCompleteQuery tc = {
					.us = us
				};
				us_tab_complete(&tc, us->ibuff, pos);
				if (tc.count)
				{
					if (tc.count == 1)
					{
						int span = (int)strlen(tc.results[0]) - tc.offset;
						memcpy(us->ibuff + pos, tc.results[0] + tc.offset, span);
						us->ibuff[pos + span++] = ' ';
						fwrite(us->ibuff + pos, 1, span, stdout);
						pos += span;
						len += span;
					}
					else
					{
						const bool has_more = tc.count > countof(tc.results);
						if (has_more)
						{
							tc.count--;
						}
						displaying_details = true;
						put(ANSI_CURSOR_SAVE ANSI_COLOR_GRAY "\n");
						while (tc.count--)
						{
							put("~ ");
							puts(tc.results[tc.count]);
						}
						if (has_more)
						{
							puts("  ...");
						}
						put(ANSI_COLOR_RESET ANSI_CURSOR_RESTORE);
					}
				}
				else
				{
					NOT_ALLOWED();
				}
			}
			break;

			case USCharDetails:
			{
				if (!len)
				{
					NOT_ALLOWED();
					break;
				}

				DetailsQuery dq;
				if (us_get_details(us, &dq, us->ibuff, pos))
				{
					displaying_details = true;
					put(ANSI_CURSOR_SAVE ANSI_COLOR_GRAY "\n");
					put(dq.result);
					put(ANSI_COLOR_RESET ANSI_CURSOR_RESTORE);
				}
				else
				{
					NOT_ALLOWED();
				}
			}
			break;

			case USCharUp:
			{
				char* line_start;
				history_line_len = us_log_read_last(&us->history, us->lbuff, &line_start, &history_pos);
				if (history_line_len)
				{
					len = history_line_len;
					if (pos)
					{
						printf("\33[%iD", pos);
					}
					put(ANSI_CLEAR_PROCEEDING_LINE);
					pos = len;
					fwrite(line_start, 1, len, stdout);
					memcpy(us->ibuff, line_start, len);
				}
				else
				{
					NOT_ALLOWED();
				}
			}
			break;

			case USCharDown:
			{
				char* line_start;
				history_pos -= history_line_len;
				history_line_len = us_log_read_next(&us->history, us->lbuff, &line_start, &history_pos);
				if (!history_line_len && !len)
				{
					NOT_ALLOWED();
					break;
				}
				len = history_line_len;
				if (pos)
				{
					printf("\33[%iD", pos);
				}
				put(ANSI_CLEAR_PROCEEDING_LINE);
				pos = len;
				if (history_line_len)
				{
					fwrite(line_start, 1, len, stdout);
					memcpy(us->ibuff, line_start, len);
				}
			}
			break;

			case USCharUndo:
				break;
			case USCharRedo:
				break;

			case USCharEscape:
				putchar('\r');
				put_prefix(us);
				put(ANSI_CLEAR_PROCEEDING);
				pos = 0;
				len = 0;
				history_pos = 0;
				history_line_len = 0;
				break;

			default:
				if (c < 256)
				{
					putchar(c);
					if (pos < len)
					{
						put(ANSI_CURSOR_SAVE);
						fwrite(us->ibuff + pos, 1, (size_t)len - pos, stdout);
						put(ANSI_CURSOR_RESTORE);
						memmove(us->ibuff + pos + 1, us->ibuff + pos, (size_t)len - pos);
					}
					us->ibuff[pos] = (char)c;
					pos++;
					len++;
				}
				break;
			}
			fflush(stdout);
		} while (c != 0);

		if (!len) continue;

		if (us_process_cmd(us, len) == USProcessCmdExit)
		{
			return true;
		}

		if (len)
		{
			us->ibuff[len] = 0;
			if (strcmp(us->ibuff, us->history.last_line))
			{
				strcpy(us->history.last_line, us->ibuff);
				us->ibuff[len] = '\n';
				fwrite(us->ibuff, 1, (size_t)len + 1, us->history.write);
				fflush(us->history.write);
			}
		}
	}
}
