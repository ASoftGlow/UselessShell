#include "ansi_codes.h"
#include "base.h"
#include "util.h"

int NO_COLOR;

#ifdef _WIN32
#include <Windows.h>

#elif __linux__
#include <termios.h>
#include <unistd.h>

struct termios oldt, newtw, newti;
#endif

bool is_setup = false;

void
ansi_setup(void)
{
    if (is_setup) return;

#ifdef _WIN32
    // enable ANSI escape codes
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;

    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#if CONFIG_ENABLE_UTF8
    SetConsoleOutputCP(CP_UTF8);
#endif

#elif __linux__
    tcgetattr(STDIN_FILENO, &oldt);
    newtw = oldt;
    newtw.c_lflag &= ~(ICANON | ECHO);
    newti = newtw;
    newti.c_cc[VTIME] = 0;
    newti.c_cc[VMIN] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &newtw);
#endif

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
ansi_setdown(void)
{
    if (!is_setup) return;

#ifdef _WIN32

#elif __linux__
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif

    put(ANSI_COLOR_RESET ANSI_CURSOR_STYLE_DEFAULT ANSI_CURSOR_SHOW ANSI_WRAP);
#if CONFIG_ENABLE_FULL_BUFFERED
    fflush(stdout);
#endif
    is_setup = false;
}