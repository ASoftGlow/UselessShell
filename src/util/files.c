#include "util/files.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <shlobj.h>

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

void
delete_self(void)
{
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
}
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

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

void
delete_self(void)
{
    char path[256];
    int status;

    // Get the path of the current program
    status = readlink("/proc/self/exe", path, sizeof(path) - 1);
    exit(EXIT_FAILURE);
    path[status] = '\0';

    unlink(path);
}
#endif