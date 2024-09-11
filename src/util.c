#include "util.h"
#include <ctype.h>
#include <errno.h>
#include <string.h>

#ifdef _WIN32
#include <conio.h>
#include <shlobj.h>
#define _us_getchar _getch

errno_t
get_cfg_path(_Out_writes_(US_MAX_PATH) char* buffer, _In_z_ const char* name)
{
    if (!SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, buffer))) return EIO;
    strcat(buffer, "\\.");
    strcat(buffer, name);
    strcat(buffer, "\\");

    if (CreateDirectoryA(buffer, NULL))
    {
        SetFileAttributesA(buffer, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED);
        return 0;
    }
    else
    {
        switch (GetLastError())
        {
        case ERROR_ALREADY_EXISTS: break;
        case ERROR_PATH_NOT_FOUND: return ENFILE;
        }
    }
    return EEXIST;
}

void
us_sleep(unsigned long ms)
{
    Sleep(ms);
}

errno_t
create_directory(_In_z_ const char* path)
{
    return !CreateDirectoryA(path, NULL);
}

errno_t
delete_directory(_Inout_z_ char* path)
{
    path[strlen(path) + 1] = 0;
    SHFILEOPSTRUCTA file_op = { NULL, FO_DELETE, path, NULL, FOF_NO_UI, FALSE, 0, 0 };
    return SHFileOperationA(&file_op);
}

int
get_directory_contents(_Inout_z_ char* path, _Out_writes_(max) char contents[][16], int max, bool is_dir)
{
    strcat(path, "\\*");
    WIN32_FIND_DATAA findData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    hFind = FindFirstFileA(path, &findData);
    path[strlen(path) - 2] = 0;

    if (hFind == INVALID_HANDLE_VALUE) return -1;

    int i = 0;
    while (FindNextFileA(hFind, &findData) && i < max)
    {
        if (is_dir && !(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
        if (strcmp(findData.cFileName, "..") == 0) continue;

        strcpy_s(contents[i++], 16, findData.cFileName);
    }
    if (i < max)
    {
        contents[i][0] = 0;
    }

    FindClose(hFind);
    return 0;
}

bool
us_chrdy(void)
{
    return _kbhit();
}
#else
#include <dirent.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define _us_getchar getchar

void
us_sleep(unsigned long ms)
{
    nanosleep((const struct timespec[]){ { 0, ms * 1000000L } }, NULL);
}

errno_t
create_directory(_In_z_ const char* path)
{
    return mkdir(path, 0755);
}

errno_t
delete_directory(_Inout_z_ char* path)
{
    return rmdir(path);
}

int
get_directory_contents(_Inout_z_ char* path, _Out_writes_(max) char contents[][16], int max, bool is_dir)
{
    int i = 0;
    DIR* d;
    struct dirent* dir;
    d = opendir(path);
    if (d)
    {
        while ((dir = readdir(d)) && i < max)
        {
            if (!is_dir && dir->d_type != DT_DIR) continue;
            if (strcmp(dir->d_name, "..") == 0) continue;
            if (strcmp(dir->d_name, ".") == 0) continue;

            strncpy(contents[i], dir->d_name, sizeof(contents[0]) - 1);
            contents[i++][sizeof(contents[0]) - 1] = 0;
        }
        closedir(d);
        if (i < max)
        {
            contents[i][0] = 0;
        }
    }
}

bool
us_chrdy(void)
{
    int bytesWaiting;
    return ioctl(0, FIONREAD, &bytesWaiting) == 0 && bytesWaiting;
}

errno_t
get_cfg_path(_Out_writes_(US_MAX_PATH) char* buffer, _In_z_ const char* name)
{
    const char* out_orig = buffer;
    char* home = getenv("XDG_CONFIG_HOME");
    unsigned int config_len = 0;
    if (!home)
    {
        home = getenv("HOME");
        if (!home)
        {
            // Can't find home directory
            buffer[0] = 0;
            return ENFILE;
        }
        config_len = strlen(".config/");
    }

    unsigned int home_len = strlen(home);

    memcpy(buffer, home, home_len);
    buffer += home_len;
    *buffer = '/';
    buffer++;
    if (config_len)
    {
        memcpy(buffer, ".config/", config_len);
        buffer += config_len;
        /* Make the .config folder if it doesn't already exist */
        *buffer = '\0';
        mkdir(out_orig, 0755);
    }
    strcpy(stpcpy(buffer, name), "/");

    errno = 0;
    mkdir(out_orig, 0755);
    return errno;
}

extern struct termios newtw, newti;

static inline void
enableInputWait(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &newtw);
}

static inline void
disableInputWait(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &newti);
}

static inline void
clearInput(void)
{
    disableInputWait();
    while (getchar() > 0)
        ;
    enableInputWait();
}
#endif

USChar
us_getchar(void)
{
    int c = _us_getchar();
    switch (c)
    {
    case 3:
    case 4: return USCharExit;

#ifdef _WIN32
    case 0:
    case 224:
        char b = _us_getchar();
        switch (b)
        {
        case 72:   return USCharUp;
        case 80:   return USCharDown;
        case 77:   return USCharRight;
        case 75:   return USCharLeft;

        case 115:  return USCharLeftWord;
        case 116:  return USCharRightWord;
        case -115: return USCharUpWord;
        case -111: return USCharDownWord;

        case 71:   return USCharHome;
        case 79:   return USCharEnd;

        case 83:   return USCharDelete;
        case -109: return USCharDeleteWord;

        case 73:   return USCharPageUp;
        case 81:   return USCharPageDown;
        case -122: return USCharPageUpWord;
        case 118:  return USCharPageDownWord;
        case -108: return USCharDetails;

        default:   return USCharUnknown;
        }
        break;
#else
    case 27:
        disableInputWait();
        c = getchar();
        enableInputWait();
        if (c < 0) return 27;
        if (c == '[')
        {
            c = getchar();
            switch (c)
            {
            case 'A': return USCharUp;
            case 'B': return USCharDown;
            case 'C': return USCharRight;
            case 'D': return USCharLeft;
            case 'H': return USCharHome;
            case 'F': return USCharEnd;
            case 'Z': return USCharDetails;

            case '3':
                switch (c = getchar())
                {
                case '~': return USCharDelete;
                case ';':
                    switch (c = getchar())
                    {
                    case '5':
                        switch (c = getchar())
                        {
                        case '~': return USCharDeleteWord;
                        }
                    }
                }

            case '1':
                switch (c = getchar())
                {
                case ';':
                    switch (c = getchar())
                    {
                    case '5':
                        switch (c = getchar())
                        {
                        case 'A': return USCharUpWord;
                        case 'B': return USCharDownWord;
                        case 'C': return USCharRightWord;
                        case 'D': return USCharLeftWord;
                        }
                    }
                }

            case '5':
                switch (c = getchar())
                {
                case '~': return USCharPageUp;
                case ';':
                    switch (c = getchar())
                    {
                    case '5':
                        switch (c = getchar())
                        {
                        case '~': return USCharPageUpWord;
                        }
                    }
                }

            case '6':
                switch (c = getchar())
                {
                case '~': return USCharPageDown;
                case ';':
                    switch (c = getchar())
                    {
                    case '6':
                        switch (c = getchar())
                        {
                        case '~': return USCharPageDownWord;
                        }
                    }
                }
            }
            clearInput();
            return USCharUnknown;
        }
        else
        {
            ungetc(c, stdin);
        }
        break;
#endif

    default: return c;
    }
}

bool
strislwr(_In_z_ const char* str)
{
    int c = 0;
    while (str[c])
    {
        if (isalpha(str[c]) && !islower(str[c])) return false;
        c++;
    }
    return true;
}

static char word_delimiters[] = " /\\()\"'-.,:;<>~!@#$%^&*|+=[]{}?";

bool
iswordend(const char c)
{
    return strchr(word_delimiters, c);
}

int
strcatc(_Inout_z_ char* str, char c)
{
    size_t len = strlen(str);
    str[len] = c;
    str[len + 1] = 0;
    return 0;
}

void
delete_self(void)
{
#ifdef _WIN32
    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    PROCESS_INFORMATION pi;
    char cmd_fmt[] = "cmd.exe /C "                                  // start seperate cmd process
                     "FOR / L % %L IN(0, 1, 10) DO "                // 10 attempts to
                     "Del \"%s\" && IF NOT EXIST \"%s\" (exit /b) " // delete file
                     "ELSE timeout /NOBREAK 1";                     // otherwise wait
    char cmd[US_MAX_PATH * 2 + sizeof(cmd_fmt)];
    sprintf(cmd, cmd_fmt, __argv[0], __argv[0]);
    CreateProcessA(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
#else
    char path[256];
    int status;

    // Get the path of the current program
    status = readlink("/proc/self/exe", path, sizeof(path) - 1);
    exit(EXIT_FAILURE);
    path[status] = '\0';

    unlink(path);
#endif
}

#ifdef WIN32
_Ret_z_ char*
stpcpy(_Inout_z_ char* dest, _In_z_ const char* src)
{
    const size_t len = strlen(src);
    return (char*)memcpy(dest, src, len + 1) + len;
}
#endif