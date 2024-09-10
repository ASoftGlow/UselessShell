#pragma once
#include "useless_shell.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define STYLE_CMD(name) ANSI_BOLD "|" ANSI_COLOR_CYAN STR_VALUE(name) ANSI_COLOR_RESET ANSI_BOLD "|" ANSI_COLOR_RESET
#define STYLE_ERROR_START "! " ANSI_COLOR_RED
#define STYLE_ERROR_END ANSI_COLOR_RESET " !"
#define STYLE_ERROR(err) STYLE_ERROR_START err STYLE_ERROR_END
#define STYLE_ARG ANSI_COLOR_GRAY
#define NOT_ALLOWED() putchar('\a')

#define USInputBufferSize (1 << 8)

_Return_type_success_(return == USReturnNormal)
ENUM(USReturn, byte,
	Normal,
	Cancel,
	Exit
);

ENUM_SLIM(USCommandReturn,
	Normal,
	Error,
	Exit
);

ENUM_REFLECT(USProcessCmd, byte,
	Success,
	ParseError,
	ExecError,
	Exit
);

ENUM(USMode, byte,
	Destoryed,
	New,
	Created
);

typedef struct
{
	FILE* write;
	FILE* read;
	char path[US_MAX_PATH];
	char last_line[USInputBufferSize];
} USLogFile;

typedef struct
{
	char username[16];
	char password[16];
	char icon;
	union
	{
		bool exists;
		time_t creation;
	};
	time_t last_login;
	bool is_super;
	byte __serial_end__;
} User;

struct UselessShell
{
#pragma region ICommandProvider
	DYNAMIC_ARRAY(cmds, const USCommand);
#pragma endregion

	USMode mode;
	User* current_user;
	User users[8];

	char ibuff[USInputBufferSize];
	char lbuff[USInputBufferSize];
	char cfg_path[US_MAX_PATH];
	USLogFile history;
};

bool
us_create(_Out_ UselessShell* us, _In_reads_(cmds_len) const USCommand* cmds, int16_t cmds_len);

void
us_destroy(_Inout_ UselessShell* us);

bool
us_start(_Inout_ UselessShell* us);

_Check_return_ USProcessCmd
us_process_cmd(_Inout_ UselessShell* us, int len);

_Ret_maybenull_ User*
us_create_user(_Inout_ UselessShell* us, _In_z_ const char* username, _In_z_ const char* password, bool is_super, char icon);

_Ret_maybenull_ User*
us_create_user_h(_Inout_ UselessShell* us, _In_z_ const char* username, _In_ const char* password_hash, bool is_super, char icon);

bool
us_delete_user(_Inout_ UselessShell* us, _In_ User* user);

bool
us_login(_Inout_ UselessShell* us, _In_ User* user, bool quiet);

bool
us_logout(_Inout_ UselessShell* us);

_Check_return_ USReturn
us_get_secret(_Out_writes_(max) char* buffer, byte min, byte max);

_Ret_maybenull_ User*
us_get_user(_In_ UselessShell* us, _In_z_ const char* username);