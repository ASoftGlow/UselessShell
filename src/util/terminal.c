#include "util/terminal.h"
#include "base.h"
#include "util/escape_codes.h"
#include "util/output.h"

#include <stdio.h>
#include <stdlib.h>

int NO_COLOR;

#ifdef _WIN32
#include <Windows.h>

DWORD dwMode;
UINT CP;

inline void
_terminal_setup(void)
{
    // enable ANSI escape codes
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    dwMode = 0;

    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#if CONFIG_ENABLE_UTF8
    CP = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);
#endif
}

inline void
_terminal_setdown(void)
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleMode(hOut, dwMode);
    if (CP) SetConsoleOutputCP(CP);
}

#elif __linux__
#include <termios.h>
#include <unistd.h>

struct termios oldt, newtw, newti;

inline void
_terminal_setup()
{
    tcgetattr(STDIN_FILENO, &oldt);
    newtw = oldt;
    newtw.c_lflag &= ~(ICANON | ECHO);
    newti = newtw;
    newti.c_cc[VTIME] = 0;
    newti.c_cc[VMIN] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &newtw);
}

inline void
_terminal_setdown()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}
#endif

bool is_setup = false;

void
terminal_setup(void)
{
    if (is_setup) return;

    _terminal_setup();

#if CONFIG_ENABLE_FULL_BUFFERED
    setvbuf(stdout, NULL, _IOFBF, (size_t)1 << 9);
#endif

    put(ANSI_COLOR_RESET ANSI_CURSOR_SHOW ANSI_WINDOW_TITLE("Useless Shell"));
#if CONFIG_ENABLE_FULL_BUFFERED
    fflush(stdout);
#endif
    NO_COLOR = !!getenv("NO_COLOR");
    is_setup = true;
}

void
terminal_setdown(void)
{
    if (!is_setup) return;

    _terminal_setdown();

    put(ANSI_COLOR_RESET ANSI_CURSOR_STYLE_DEFAULT ANSI_CURSOR_SHOW ANSI_WRAP);
#if CONFIG_ENABLE_FULL_BUFFERED
    fflush(stdout);
#endif
    is_setup = false;
}