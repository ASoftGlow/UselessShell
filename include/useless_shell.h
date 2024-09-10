#pragma once
#include "base.h"

#include <stddef.h>

#define US_MAX_PATH 260
#define DYNAMIC_ARRAY(name, type) \
    type* name;                   \
    int16_t name##_len

typedef struct UselessShell UselessShell;

ENUM(USCommandArgType, byte,
	Unset,
	Number,
	String,
	Secret,
	Boolean,
	Flag
);
typedef byte USCommandArgType;

typedef struct
{
    const UselessShell* us;

    const char* part;
    const char* results[2];
    byte count;
    byte offset;

    char parts[32 * 16];
    bool is_whitespace_terminated;
} USTabCompleteQuery;

typedef struct
{
    char name[16];
    char description[64];
    USCommandArgType type;
    bool optional;
    void (*auto_complete)(USTabCompleteQuery* tb);
} USCommandArg;

typedef struct
{
    union
    {
        int64_t number;
        char* string;
        bool boolean;
        size_t exists;
    };
} USCommandArgValue;

typedef byte USCommandReturn;

typedef struct _Command
{
#pragma region ICommandProvider
    DYNAMIC_ARRAY(subcmds, struct _Command);
#pragma endregion

    char name[16];
    char description[64];
    bool is_super;

    USCommandReturn (*impl)(UselessShell* us, USCommandArgValue* args);

    DYNAMIC_ARRAY(args, USCommandArg);
} USCommand;

_Ret_maybenull_ UselessShell* useless_shell_create(_In_reads_(cmds_len) const USCommand* cmds, int16_t cmds_len);
void useless_shell_destroy(_Post_invalid_ _In_ UselessShell* us);
bool useless_shell_start(_Inout_ UselessShell* us);
bool useless_shell_create_user(
    _Inout_ UselessShell* us, _In_z_ const char* username, _In_z_ const char* password, bool is_super, char icon
);
bool useless_shell_delete_user(_Inout_ UselessShell* us, _In_z_ const char* username);
bool useless_shell_login(_Inout_ UselessShell* us, _In_z_ const char* username);
bool useless_shell_logout(_Inout_ UselessShell* us);