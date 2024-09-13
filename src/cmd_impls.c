#include "cmd_impls.h"
#include "us_impl.h"

#include "util/escape_codes.h"
#include "util/files.h"
#include "util/input.h"
#include "util/misc.h"
#include "util/output.h"
#include "util/string.h"

CMD_IMPL_PROTO(clear)
{
    put(ANSI_CLEAR_OUTPUT);
    return USCommandReturnNormal;
}

static void
put_help(const USCommand* cmd, byte indent, bool is_brief)
{
    for (byte i = 0; i < indent; i++) put("  ");
    put("- ");
    put(cmd->name);
    for (int a = 0; a < cmd->args_len; a++)
    {
        USCommandArg* arg = &cmd->args[a];
        putchar(' ');
        putchar(arg->type == USCommandArgTypeFlag ? '-' : arg->optional ? '(' : '<');
        put(arg->name);
        if (arg->type != USCommandArgTypeFlag) putchar(arg->optional ? ')' : '>');
    }
    if (*cmd->description && !is_brief)
    {
        for (byte i = 0; i < indent; i++) put("  ");
        put("\n  " ANSI_COLOR_GRAY);
        put(cmd->description);
        puts(ANSI_COLOR_RESET);
    }
    else
    {
        putchar('\n');
    }

    for (int i = 0; i < cmd->subcmds_len; i++)
    {
        put_help(&cmd->subcmds[i], indent + 1, is_brief);
    }
}

#include <string.h>

CMD_IMPL_PROTO(help)
{
    bool is_brief = args[1].exists;
    if (args[0].exists)
    {
        for (int i = 0; i < us->cmds_len; i++)
        {
            if (strcmp(us->cmds[i].name, args[0].string) == 0)
            {
                put_help(&us->cmds[i], 0, is_brief);
                return USCommandReturnNormal;
            }
        }
        put(STYLE_ERROR_START "Unknown command " STYLE_ARG);
        put(args[0].string);
        puts(STYLE_ERROR_END);
        return USCommandReturnError;
    }
    else
    {
        for (int c = 0; c < us->cmds_len; c++)
        {
            put_help(&us->cmds[c], 0, is_brief);
        }
    }
    return USCommandReturnNormal;
}

CMD_IMPL_PROTO(history)
{
    if (fseek(us->history.read, 0, SEEK_SET) == 0)
    {
        char a;
        while ((a = fgetc(us->history.read)) != EOF)
        {
            putchar(a);
        }
    }
    else
    {
        puts(STYLE_ERROR("Failed to read history"));
    }
    return USCommandReturnNormal;
}

CMD_IMPL_PROTO(history_clear)
{
    if (freopen(us->history.path, "w", us->history.write))
    {
        if (freopen(us->history.path, "a", us->history.write))
        {
            setvbuf(us->history.write, NULL, _IONBF, 0);
        }
    }
    return USCommandReturnNormal;
}

#include "Md5.h"

CMD_IMPL_PROTO(login)
{
    const char* username = args[0].string;
    put("Enter password: ");
    fflush(stdout);
    char password[20];
    switch (us_get_secret(password, 3, sizeof(password)))
    {
    case USReturnNormal:
        User* user = us_get_user(us, username);
        MD5_CTX md5;
        MD5Init(&md5);
        MD5Update(&md5, password, strlen(password));
        MD5Final(&md5);
        if (user && memcmp(md5.digest, user->password, sizeof(user->password)) == 0)
        {
            us_login(us, user, false);
        }
        else
        {
            puts(STYLE_ERROR("Incorrect password"));
            NOT_ALLOWED();
            fflush(stdout);
            us_sleep(1000);
        }
        break;

    case USReturnCancel: break;

    case USReturnExit:   return USCommandReturnExit;
    }
    return USCommandReturnNormal;
}

CMD_IMPL_PROTO(logout)
{
    us_logout(us);
    return USCommandReturnNormal;
}

#include <time.h>

static void
us_user_info(User* user)
{
    put("Username:\t");
    puts(user->username);
    put("Created:\t");
    const char* ti = asctime(localtime(&user->creation));
    if (!ti) return;
    put(ti);
    put("Last login:\t");
    ti = asctime(localtime(&user->creation));
    if (!ti) return;
    put(ti);
    put("Icon:\t\t");
    putchar(user->icon);
    put("\nSuper:\t\t");
    puts(user->is_super ? "true" : "false");
}

CMD_IMPL_PROTO(user_info)
{
    if (args[0].exists)
    {
        User* user = us_get_user(us, args[0].string);
        if (user)
        {
            us_user_info(user);
        }
        else
        {
            puts(STYLE_ERROR("User doesn't exist"));
        }
    }
    else if (us->current_user)
    {
        us_user_info(us->current_user);
    }
    else
    {
        puts("Guest.");
    }
    return USCommandReturnNormal;
}

CMD_IMPL_PROTO(user_list)
{
    int c = 0;
    for (int i = 0; i < countof(us->users); i++)
    {
        User* user = &us->users[i];
        if (user->exists)
        {
            printf("%i\t[%c] %s\n", ++c, user->icon, user->username);
        }
    }
    return USCommandReturnNormal;
}

CMD_IMPL_PROTO(user_create)
{
    const bool is_super = args[2].exists ? args[2].boolean : false;
    if (!(is_super && us->current_user && us->current_user->is_super))
    {
        puts(STYLE_ERROR("Insufficient permission"));
        return USCommandReturnError;
    }
    if (useless_shell_create_user(us, args[0].string, args[1].string, is_super, 'A'))
    {
        printf("Created user %s.\n", args[0].string);
        return USCommandReturnNormal;
    }
    return USCommandReturnError;
}

CMD_IMPL_PROTO(user_delete)
{
    if (!useless_shell_delete_user(us, args[0].string))
    {
        puts(STYLE_ERROR("Failed to delete user"));
        return USCommandReturnError;
    }
    return USCommandReturnNormal;
}

CMD_IMPL_PROTO(exit) { return USCommandReturnExit; }

CMD_IMPL_PROTO(uninstall)
{
    us_destroy(us);
    delete_directory(us->cfg_path);
    delete_self();
    return USCommandReturnExit;
}

static const char* digital_numbers[][3] = {
    {"|_|",  "| |", " _ "},
    { "_|_", " | ", "_  "},
    { "|__", " _|", "__ "},
    { "__|", " _|", "__ "},
    { "  |", "|_|", "   "},
    { "__|", "|_ ", " __"},
    { "|_|", "|_ ", " __"},
    { "  |", "  |", "__ "},
    { "|_|", "|_|", " _ "},
    { "__|", "|_|", " _ "},
    { " . ", " ' ", "   "}
};

static void
put_digital(_Field_range_(0, 10) byte n, byte row)
{
    if (n < 10)
    {
        put(digital_numbers[0][row]);
        putchar(' ');
        put(digital_numbers[n][row]);
    }
    else
    {
        put(digital_numbers[n / 10][row]);
        putchar(' ');
        put(digital_numbers[n % 10][row]);
    }
}

CMD_IMPL_PROTO(time)
{
    const bool is_digital = args[0].exists && args[0].boolean;
    const bool watch = args[1].exists && args[1].boolean;

    if (watch)
    {
        put(ANSI_CURSOR_HIDE);
    }
    put(ANSI_CURSOR_SAVE);

loop:
    const time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    if (is_digital)
    {
        for (int8_t row = countof(digital_numbers[0]) - 1; row >= 0; row--)
        {
            put_digital(tm->tm_hour, row);
            put(digital_numbers[10][row]);
            put_digital(tm->tm_min, row);
            put(digital_numbers[10][row]);
            put_digital(tm->tm_sec, row);
            putchar('\n');
        }
    }
    else
    {
        printf("%02i:%02i:%02i\n", tm->tm_hour, tm->tm_min, tm->tm_sec);
    }

    if (watch)
    {
        fflush(stdout);
        us_sleep(1000);
        put("\r");
        put(ANSI_CURSOR_RESTORE);
        if (!us_chrdy())
        {
            goto loop;
        }
        (void)us_getchar();
        put("\n" ANSI_CURSOR_SHOW);
        if (is_digital) put("\n\n");
    }
    return USCommandReturnNormal;
}

#pragma region Auto Completes

AC_IMPL_PROTO(users)
{
    tc->count = 0;
    for (int i = 0; i < countof(tc->us->users); i++)
    {
        const User* user = &tc->us->users[i];
        if (user->exists && strncmp(tc->part, user->username, strlen(tc->part)) == 0)
        {
            if (tc->count == countof(tc->results))
            {
                tc->count++;
                break;
            }
            tc->results[tc->count++] = user->username;
        }
    }
}

AC_IMPL_PROTO(cmds)
{
    tc->count = 0;
    for (short i = 0; i < tc->us->cmds_len; i++)
    {
        const USCommand* cmd = &tc->us->cmds[i];
        if (strncmp(cmd->name, tc->part, strlen(tc->part)) == 0)
        {
            if (tc->count == countof(tc->results))
            {
                tc->count++;
                break;
            }
            tc->results[tc->count++] = cmd->name;
        }
    }
}

#pragma endregion